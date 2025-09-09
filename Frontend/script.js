const statusText = document.getElementById('status-text');
const statusTipo = document.getElementById('status-tipo');
const statusEnvio = document.getElementById('status-envio');
const statusEnvioSM = document.getElementById('status-envio-sm');
const selectElement = document.getElementById('opcoes');
const nameProcess1 = document.getElementById('name_process_1');
const nameProcess2 = document.getElementById('name_process_2');
const painelPadrao = document.getElementById('painel-padrao');
const painelSharedMemory = document.getElementById('painel-sharedmemory');

// Mapeamento dos nomes de exibição de cada processo de acordo com a opção escolhida
const nomesProcessos = {
    pipes: {
        processo1: "Remetente",
        processo2: "Destinatário"
    },
    sockets: {
        processo1: "Cliente",
        processo2: "Servidor"
    },
    sharedmemory: {
        processo1: "Processo A",
        processo2: "Processo B"
    }
};

// Função para atualizar a interface de status (ligado/desligado + tipo de processo)
function updateUI(status) {
    if (status === 'on') {
        statusText.textContent = 'Ligado';
        statusText.className = 'status-on';
        if (selectElement.value === 'pipes'){
          statusTipo.textContent = ' (Pipes) ';
        } else if (selectElement.value === 'sockets'){
          statusTipo.textContent = ' (Sockets) ';
        } else if (selectElement.value === 'sharedmemory'){
          statusTipo.textContent = ' (Memória Compartilhada) ';
        }
        
    } else {
        statusText.textContent = 'Desligado';
        statusText.className = 'status-off';
        statusTipo.textContent = ' (Nenhum processo iniciado) '
    }
}

// Evento que troca os nomes/processos exibidos e o painel mostrado conforme a opção escolhida
selectElement.addEventListener('change', () => {
    const selectedOption = selectElement.value;
    const nomes = nomesProcessos[selectedOption];
    if (nomes) {
        nameProcess1.textContent = nomes.processo1;
        nameProcess2.textContent = nomes.processo2;
    }
    // Mostra o painel correto de acordo com o tipo selecionado
    if (selectedOption === 'sharedmemory') {
        painelPadrao.style.display = 'none';
        painelSharedMemory.style.display = 'block';
    } else {
        painelPadrao.style.display = 'block';
        painelSharedMemory.style.display = 'none';
    }
});

// Quando a página carrega, recupera os valores do localStorage para manter o estado
document.addEventListener('DOMContentLoaded', () => {
    // Tenta obter o status salvo. Se não existir, usa 'off' como padrão.
    const savedStatus = localStorage.getItem('processStatus') || 'off';
    // Tenta obter o tipo de processo salvo. Se não existir, usa 'pipes' como padrão.
    const savedType = localStorage.getItem('processType') || 'pipes';
    // Define o valor do <select> com o tipo de processo salvo
    selectElement.value = savedType;
    // Atualiza a interface
    updateUI(savedStatus);
    const selectedOption = selectElement.value;
    const nomes = nomesProcessos[selectedOption];
    if (nomes) {
        nameProcess1.textContent = nomes.processo1;
        nameProcess2.textContent = nomes.processo2;
    }
    // Mostra/esconde painel correto logo na inicialização
    if (selectedOption === 'sharedmemory') {
        painelPadrao.style.display = 'none';
        painelSharedMemory.style.display = 'block';
    } else {
        painelPadrao.style.display = 'block';
        painelSharedMemory.style.display = 'none';
    }
});

// Botão iniciar
document.querySelector(".processo_start_button").addEventListener("click", async () => {
    console.log("Cliquei em iniciar"); // log de teste
    const selectedOption = selectElement.value; // Captura o valor selecionado
    try {
    // Chama o backend para iniciar o processo com a opção escolhida
    const res = await fetch(`http://localhost:3000/start-processo/${selectedOption}`);
    const dados = await res.json();
    console.log(dados.status);
    
    // Obtém o status atual do localStorage antes de fazer qualquer coisa
    const currentStatus = localStorage.getItem('processStatus');

    // **Verifica se o processo está desligado antes de iniciar a transição**
    if (currentStatus === 'off' || !currentStatus) { // A segunda condição é para a primeira vez que a página carrega
        // Muda o texto para 'Iniciando...' imediatamente
        statusText.textContent = 'Iniciando...';
        statusText.className = 'status-iniciando';
    }
    
    // Define o status como 'on'
    const newStatus = 'on';

    // Se a resposta do servidor indicar sucesso, atualiza a UI.
    if (dados.status.includes("iniciados!")) {
      const newStatus = 'on';
      // Salva o novo status no localStorage
      // Usa setTimeout para atrasar a atualização da interface
      setTimeout(() => {
        localStorage.setItem('processStatus', newStatus);
        localStorage.setItem('processType', selectedOption);
        statusEnvio.textContent = dados.status;
        statusEnvioSM.textContent = dados.status;
        updateUI(newStatus);
      }, 1800);// 1800 milissegundos = 1.8 segundos
    } else {
      // Se o servidor retornou um erro (ex: "Um processo já está rodando.")
      // Apenas atualiza a mensagem de status de envio e não altera o status geral.
        statusEnvio.textContent = dados.status;
        statusEnvioSM.textContent = dados.status;
    }

  } catch (err) {
    statusEnvio.textContent = "Erro ao iniciar processos.";
    statusEnvioSM.textContent = "Erro ao iniciar processos.";
    console.error("Erro ao iniciar processos:", err);
  }
});

// Botão finalizar
document.querySelector(".processo_stop_button").addEventListener("click", async () => {
  const selectedOption = selectElement.value; // Captura o valor selecionado

  try {
    // Envia a opção selecionada na URL para o servidor
    const res = await fetch(`http://localhost:3000/stop-processo/${selectedOption}`);
    await limparLog(); // limpa os logs locais
    const dados = await res.json();
    // Pega a resposta do backend (dados.status) e loga no console
    console.log(dados.status);
    statusEnvio.textContent = dados.status; // Atualiza a mensagem de status das ações 
    statusEnvioSM.textContent = dados.status;
    // Define o status como 'off'
    const newStatus = 'off';
    // Salva o novo status no localStorage
    localStorage.setItem('processStatus', newStatus);
    // Atualiza a interface
    updateUI(newStatus);

  } catch (err) {
    statusEnvio.textContent = "Erro ao finalizar processos.";
    statusEnvioSM.textContent = "Erro ao finalizar processos.";
    console.error("Erro ao finalizar processos:", err);
  }
});

// Botão de Enviar Mensagem
document.querySelector(".enviar_msg_button").addEventListener("click", async () => {
  const inputElement = document.querySelector(".input");
  const mensagem = inputElement.value;

  // bloqueia envio vazio
  if (mensagem.trim() === "") {
     statusEnvio.textContent = 'A mensagem não pode ser vazia.';
     statusEnvioSM.textContent = 'A mensagem não pode ser vazia.';
     return; // Não envia mensagem vazia
  }
  // Quando clica em Enviar, ele faz um fetch (requisição HTTP) para a rota do backend enviar-mensagem.
  try {
    const res = await fetch("http://localhost:3000/enviar-mensagem", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      // Aqui só tem o envio de uma mensagem por vez, enviada pelo Remetente ou Cliente (Processo 1).
      // O backend repassa pro processo 2 (Servidor).
      body: JSON.stringify({ mensagem: mensagem }),
    });
    // O servidor responde e o res.json() transforma a resposta em JSON pra ser usado no front.
    const dados = await res.json();
    console.log(dados.status);
    inputElement.value = ""; // Limpa o input após o envio
    statusEnvio.textContent = dados.status;
    statusEnvioSM.textContent = dados.status;
  } catch (err) {
    statusEnvio.textContent = "Erro ao enviar mensagem.";
    statusEnvioSM.textContent = 'A mensagem não pode ser vazia.';
    console.error("Erro ao enviar mensagem:", err);
  }
});

// Botão Enviar Mensagem (painel memória compartilhada)
document.querySelector(".enviar_msg_button_sm").addEventListener("click", async () => {
    // Lógica para enviar as mensagens A e B para o backend.
    const msgA = document.querySelector(".input-A").value;
    const msgB = document.querySelector(".input-B").value;
    
    // Impede envio se os dois campos estiverem vazios
    if (msgA.trim() === "" && msgB.trim() === "") {
        statusEnvioSM.textContent = 'Pelo menos uma das mensagens deve ser preenchida.';
        return;
    }
  
    // Quando clica em Enviar, ele faz um fetch (requisição HTTP) para a rota do backend enviar-mensagem-sm.
    try {
      const res = await fetch("http://localhost:3000/enviar-mensagem-sm", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        // Ele envia duas mensagens de uma vez (mensagemA e mensagemB), porque na memória compartilhada os dois processos podem escrever simultaneamente.
        body: JSON.stringify({ mensagemA: msgA, mensagemB: msgB }),
      });
      // O servidor responde e o res.json() transforma a resposta em JSON pra ser usado no front.
      const dados = await res.json();
      console.log(dados.status);
      // limpa os inputs após enviar
      document.querySelector(".input-A").value = "";
      document.querySelector(".input-B").value = "";
      statusEnvioSM.textContent = dados.status;
      
    } catch (err) {
      statusEnvioSM.textContent = "Erro ao enviar mensagem.";
      console.error("Erro ao enviar mensagem:", err);
    }
});

// Botão Limpar Log (painel memória compartilhada)
document.querySelector(".limpar_log_button_sm").addEventListener("click", async () => {
    // Lógica para limpar os inputs e o log
    await limparLog();
    document.querySelector(".input-A").value = "";
    document.querySelector(".input-B").value = "";
});

// Botão de Limpar Log na tela e no servidor
async function limparLog() {
  const proc1 = document.querySelector(".section-process_1_content");
  const proc2 = document.querySelector(".section-process_2_content");
  
  // Apaga o conteúdo visualmente
  // Remove mensagens do front (deixa só o título <h1>)
  while (proc1.children.length > 1) {
      proc1.removeChild(proc1.lastChild);
  }
  while (proc2.children.length > 1) {
      proc2.removeChild(proc2.lastChild);
  }
  
  try {
      // Envia uma requisição ao servidor para limpar a memória
      const res = await fetch("http://localhost:3000/limpar-mensagens", {
          method: "POST"
      });
      const dados = await res.json();
      console.log(dados.status);
      ultimaQuantidade = 0; // Reseta o contador
      statusEnvio.textContent = dados.status;
      statusEnvioSM.textContent = dados.status;
  } catch (err) {
      statusEnvio.textContent = "Erro ao limpar mensagens do servidor.";
      statusEnvioSM.textContent = "Erro ao limpar mensagens do servidor.";
      console.error("Erro ao limpar mensagens no servidor:", err);
  }
}

// guarda a quantidade de mensagens já exibidas
let ultimaQuantidade = 0;

// Função que carrega mensagens do servidor e exibe na tela
async function carregarMensagens() {
  try {
    const res = await fetch("http://localhost:3000/mensagens");
    const dados = await res.json(); // deve retornar array

    // Só adiciona mensagens novas
    if (Array.isArray(dados) && dados.length > ultimaQuantidade) {
      const novos = dados.slice(ultimaQuantidade); // pega só o que é novo
      ultimaQuantidade = dados.length;

      // Pega o tipo de processo atualmente selecionado
      const tipoProcesso = selectElement.value;
      const nomes = nomesProcessos[tipoProcesso];

      novos.forEach(msg => {
        const proc1 = document.querySelector(".section-process_1_content");
        const proc2 = document.querySelector(".section-process_2_content");
        const span = document.createElement("span");
        span.classList.add("text", "subtitle");

        // Para cada mensagem nova vinda do servidor, ele cria um elemento <span> e coloca o texto dentro.
        if (msg.processo === 1) {
          span.innerText = `${nomes.processo1}: ${msg.mensagem}`;
          proc1.appendChild(span);
          proc1.scrollTop = proc1.scrollHeight; // scroll automático
        }
        if (msg.processo === 2) {
          span.innerText = `${nomes.processo2}: ${msg.mensagem}`;
          proc2.appendChild(span);
          proc2.scrollTop = proc2.scrollHeight; // scroll automático
        }
      });
    }
  } catch (err) {
    statusEnvio.textContent = "Erro ao carregar mensagens";
    statusEnvioSM.textContent = "Erro ao carregar mensagens";
    console.error("Erro ao carregar mensagens:", err);
  }
}

// scroll automático
setInterval(carregarMensagens, 1500);
carregarMensagens();

// Botão Limpar Log (painel padrão)
document.querySelector(".limpar_log_button").addEventListener("click", limparLog);
