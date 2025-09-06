const statusText = document.getElementById('status-text');
const statusEnvio = document.getElementById('status-envio');
// Função para atualizar a interface (texto e cor)
function updateUI(status) {
    if (status === 'on') {
        statusText.textContent = 'Ligado';
        statusText.className = 'status-on';
    } else {
        statusText.textContent = 'Desligado';
        statusText.className = 'status-off';
    }
}

// Verifica o estado salvo no localStorage quando a página é carregada
document.addEventListener('DOMContentLoaded', () => {
    // Tenta obter o status salvo. Se não existir, usa 'off' como padrão.
    const savedStatus = localStorage.getItem('processStatus') || 'off';
    updateUI(savedStatus);
});

// Botão iniciar
document.querySelector(".processo_start_button").addEventListener("click", async () => {
   console.log("Cliquei em iniciar"); // 👈 testando
   try {
    const res = await fetch("http://localhost:3000/start-sockets");
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

    // Salva o novo status no localStorage
    // Usa setTimeout para atrasar a atualização da interface
    setTimeout(() => {
        // Salva o novo status no localStorage
        localStorage.setItem('processStatus', newStatus);
        statusEnvio.textContent = dados.status;
        // Atualiza a interface com o novo status
        updateUI(newStatus);
    }, 1800); // 1500 milissegundos = 1.5 segundos

  } catch (err) {
    statusEnvio.textContent = "Erro ao iniciar processos.";
    console.error("Erro ao iniciar processos:", err);
  }
});

// Botão finalizar
document.querySelector(".processo_stop_button").addEventListener("click", async () => {
  try {
    const res = await fetch("http://localhost:3000/stop-sockets");
    const dados = await res.json();
    console.log(dados.status);
    statusEnvio.textContent = dados.status;
    // Define o status como 'off'
    const newStatus = 'off';
    // Salva o novo status no localStorage
    localStorage.setItem('processStatus', newStatus);
    // Atualiza a interface
    updateUI(newStatus);

  } catch (err) {
    statusEnvio.textContent = "Erro ao finalizar processos.";
    console.error("Erro ao finalizar processos:", err);
  }
});

// Botão de Enviar Mensagem
document.querySelector(".enviar_msg_button").addEventListener("click", async () => {
  const inputElement = document.querySelector(".input");
  const mensagem = inputElement.value;

  if (mensagem.trim() === "") {
     statusEnvio.textContent = 'A mensagem não pode ser vazia.';
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
  } catch (err) {
    statusEnvio.textContent = "Erro ao enviar mensagem.";
    console.error("Erro ao enviar mensagem:", err);
  }
});

// Botão de Limpar Log
document.querySelector(".limpar_log_button").addEventListener("click", async () => {
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
        ultimaQuantidade = 0; // CORREÇÃO: Reseta o contador
        statusEnvio.textContent = dados.status;
    } catch (err) {
        statusEnvio.textContent = "Erro ao limpar mensagens do servidor.";
        console.error("Erro ao limpar mensagens no servidor:", err);
    }
});

let ultimaQuantidade = 0;

async function carregarMensagens() {
  try {
    const res = await fetch("http://localhost:3000/mensagens");
    const dados = await res.json(); // agora é array

    if (Array.isArray(dados) && dados.length > ultimaQuantidade) {
      const novos = dados.slice(ultimaQuantidade); // pega só o que é novo
      ultimaQuantidade = dados.length;

      novos.forEach(msg => {
        const proc1 = document.querySelector(".section-process_1_content");
        const proc2 = document.querySelector(".section-process_2_content");
        const span = document.createElement("span");
        span.classList.add("text", "subtitle");

        if (msg.processo === 1) {
          span.innerText = "Cliente: " + msg.mensagem;
          proc1.appendChild(span);
          proc1.scrollTop = proc1.scrollHeight;
        } else if (msg.processo === 2) {
          span.innerText = "Servidor: " + msg.mensagem;
          proc2.appendChild(span);
          proc2.scrollTop = proc2.scrollHeight;
        }
      });
    }
  } catch (err) {
    statusEnvio.textContent = "Erro ao carregar mensagens";
    console.error("Erro ao carregar mensagens:", err);
  }
}

// Atualiza a cada 1.5s
setInterval(carregarMensagens, 1500);
carregarMensagens();

