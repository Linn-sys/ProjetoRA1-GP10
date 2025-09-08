const statusText = document.getElementById('status-text');
const statusTipo = document.getElementById('status-tipo');
const statusEnvio = document.getElementById('status-envio');
const statusEnvioSM = document.getElementById('status-envio-sm');
const selectElement = document.getElementById('opcoes');
const nameProcess1 = document.getElementById('name_process_1');
const nameProcess2 = document.getElementById('name_process_2');
const painelPadrao = document.getElementById('painel-padrao');
const painelSharedMemory = document.getElementById('painel-sharedmemory');

// Mapeamento dos nomes de exibição
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

// Função para atualizar a interface (texto e cor)
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

// Adicionar um ouvinte de evento para quando a opção for alterada
selectElement.addEventListener('change', () => {
    const selectedOption = selectElement.value;
    const nomes = nomesProcessos[selectedOption];
    if (nomes) {
        nameProcess1.textContent = nomes.processo1;
        nameProcess2.textContent = nomes.processo2;
    }
    // Lógica para mostrar o painel correto
    if (selectedOption === 'sharedmemory') {
        painelPadrao.style.display = 'none';
        painelSharedMemory.style.display = 'block';
    } else {
        painelPadrao.style.display = 'block';
        painelSharedMemory.style.display = 'none';
    }
});

// Verifica o estado salvo no localStorage quando a página é carregada
document.addEventListener('DOMContentLoaded', () => {
    // Tenta obter o status salvo. Se não existir, usa 'off' como padrão.
    const savedStatus = localStorage.getItem('processStatus') || 'off';
    // Tenta obter o tipo de processo salvo. Se não existir, usa 'pipes' como padrão.
    const savedType = localStorage.getItem('processType') || 'pipes';
    // Define o valor do <select> com o tipo de processo salvo
    selectElement.value = savedType;
    updateUI(savedStatus);
    const selectedOption = selectElement.value;
    const nomes = nomesProcessos[selectedOption];
    if (nomes) {
        nameProcess1.textContent = nomes.processo1;
        nameProcess2.textContent = nomes.processo2;
    }
    // Chama a lógica de esconder/mostrar o painel aqui também, para que ele inicie no estado correto.
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
    console.log("Cliquei em iniciar"); // testando
    const selectedOption = selectElement.value; // Captura o valor selecionado
    try {
    // Envia a opção selecionada na URL para o servidor
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
    // O servidor.js já está retornando "Um processo já está rodando."
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
    await limparLog();
    const dados = await res.json();
    console.log(dados.status);
    statusEnvio.textContent = dados.status;
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

  if (mensagem.trim() === "") {
     statusEnvio.textContent = 'A mensagem não pode ser vazia.';
     statusEnvioSM.textContent = 'A mensagem não pode ser vazia.';
     return; // Não envia mensagem vazia
  }
  try {
    const res = await fetch("http://localhost:3000/enviar-mensagem", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ mensagem: mensagem }),
    });
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

document.querySelector(".enviar_msg_button_sm").addEventListener("click", async () => {
    // Lógica para enviar as mensagens A e B para o backend.
    const msgA = document.querySelector(".input-A").value;
    const msgB = document.querySelector(".input-B").value;
    
    if (msgA.trim() === "" && msgB.trim() === "") {
        statusEnvioSM.textContent = 'Pelo menos uma das mensagens deve ser preenchida.';
        return;
    }
  
    try {
      const res = await fetch("http://localhost:3000/enviar-mensagem-sm", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ mensagemA: msgA, mensagemB: msgB }),
      });
      const dados = await res.json();
      console.log(dados.status);
      document.querySelector(".input-A").value = "";
      document.querySelector(".input-B").value = "";
      statusEnvioSM.textContent = dados.status;
      
    } catch (err) {
      statusEnvioSM.textContent = "Erro ao enviar mensagem.";
      console.error("Erro ao enviar mensagem:", err);
    }
});

document.querySelector(".limpar_log_button_sm").addEventListener("click", async () => {
    // Lógica para limpar os inputs e o log
    await limparLog();
    document.querySelector(".input-A").value = "";
    document.querySelector(".input-B").value = "";
});

// Botão de Limpar Log
async function limparLog() {
  const proc1 = document.querySelector(".section-process_1_content");
  const proc2 = document.querySelector(".section-process_2_content");
  
  // Apaga o conteúdo visualmente
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

let ultimaQuantidade = 0;

async function carregarMensagens() {
  try {
    const res = await fetch("http://localhost:3000/mensagens");
    const dados = await res.json(); // agora é array

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

        if (msg.processo === 1) {
          span.innerText = `${nomes.processo1}: ${msg.mensagem}`;
          proc1.appendChild(span);
          proc1.scrollTop = proc1.scrollHeight;
        }
        if (msg.processo === 2) {
          span.innerText = `${nomes.processo2}: ${msg.mensagem}`;
          proc2.appendChild(span);
          proc2.scrollTop = proc2.scrollHeight;
        }
      });
    }
  } catch (err) {
    statusEnvio.textContent = "Erro ao carregar mensagens";
    statusEnvioSM.textContent = "Erro ao carregar mensagens";
    console.error("Erro ao carregar mensagens:", err);
  }
}

// Atualiza a cada 1.5s
setInterval(carregarMensagens, 1500);
carregarMensagens();

document.querySelector(".limpar_log_button").addEventListener("click", limparLog);
