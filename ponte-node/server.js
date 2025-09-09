const express = require("express"); // Importa o framework Express para criar o servidor HTTP
const net = require("net"); // Importa o módulo net para comunicação via sockets
const cors = require("cors"); // Importa CORS para permitir requisições de diferentes origens
const { spawn, exec } = require("child_process"); // Importa funções para executar processos externos
const path = require("path"); // Importa path para manipulação de caminhos de arquivos

const app = express(); // Cria a aplicação Express
app.use(cors()); // Habilita CORS para todas as rotas
app.use(express.json()); // Permite que o Express interprete requisições JSON

// Armazena todas as mensagens enviadas/recebidas
let mensagens = [];
// Variáveis para controlar os processos iniciados (Remetente & Destinatário, Cliente & Servidor, Processo A  & Processo B)
let servidorProcess = null;
let clienteProcess = null;
let client = null; // conexão Node -> Servidor C++
let processoAtual = null; // Guarda o tipo de processo atualmente em execução

// Mapa para guardar os caminhos dos executáveis
const executaveis = {
  sockets: {
    servidor: path.resolve(__dirname, "../Backend/x64/Debug/ServidorSocket.exe"),
    cliente: path.resolve(__dirname, "../Backend/x64/Debug/ClienteSocket.exe"),
  },
  pipes: {
    servidor: path.resolve(__dirname, "../Backend/x64/Debug/PipeOffshoot.exe"),
    cliente: path.resolve(__dirname, "../Backend/x64/Debug/PipeMain.exe"),
  },
  sharedmemory: {
    servidor: path.resolve(__dirname, "../Backend/x64/Debug/SMProcesso1.exe"),
    cliente: path.resolve(__dirname, "../Backend/x64/Debug/SMProcesso2.exe")
  }
};

// Endpoint: o front vai buscar a lista completa de mensagens armazenadas
app.get("/mensagens", (req, res) => {
  res.json(mensagens);
});

// Iniciar os .exe
// Endpoint genérico para iniciar um processo
app.get("/start-processo/:tipo", (req, res) => {
  const tipo = req.params.tipo; // Recebe o tipo de processo via URL
  const paths = executaveis[tipo]; // Pega os caminhos dos executáveis para o tipo selecionado
  
  // Verifica se os executáveis existem
  if (!paths || !paths.servidor || !paths.cliente) {
    return res.status(400).json({ status: "Opção de processo inválida ou não implementada." });
  }

  // Verifica se já existe algum processo rodando
  if (servidorProcess || clienteProcess) {
    return res.json({ status: `Um processo já está rodando (${processoAtual}).` });
  }

  console.log(`[Node] Iniciando os processos para: ${tipo}`);
  
  // Armazena o tipo de processo iniciado
  processoAtual = tipo; 

  // -------------------------
  // Caso 1: PIPES
  // -------------------------
  if (tipo === 'pipes') {
    mensagens = []; // limpa histórico
    // Inicia o cliente (PipeMain), que por sua vez cria o destinatário
    console.log("[Node] Iniciando PipeMain (que cria o Offshoot)...");

    clienteProcess = spawn(paths.cliente, [], {
      cwd: path.dirname(paths.cliente),
      detached: false,
    });

    // Buffer para acumular dados recebidos do stdout
    let bufferPipe = "";

    // Escuta mensagens do PipeMain (remetente)
    clienteProcess.stdout.on("data", (data) => {
      bufferPipe += data.toString();
      let index;
      // Divide o buffer por linhas completas
      while ((index = bufferPipe.indexOf("\n")) >= 0) {
        const linha = bufferPipe.substring(0, index).trim();
        bufferPipe = bufferPipe.substring(index + 1);

        if (linha.length > 0) {
          try {
            // Tenta interpretar a linha como JSON
            const msg = JSON.parse(linha);
            // Adiciona a mensagem ao histórico
            mensagens.push(msg);
            console.log("[Node] Recebido do PipeMain:", msg);
          } catch (e) {
            console.error("[Node] JSON inválido do PipeMain:", linha);
          }
        }
      }
    });

    // Captura erros do processo PipeMain
    clienteProcess.stderr.on("data", (data) => {
      console.error(`[PipeMain ERRO]: ${data}`);
    });
    
  }

  // -------------------------
  // Caso 2: SOCKETS
  // -------------------------
  else if (tipo === 'sockets') {
    mensagens = []; // limpa histórico
    // Inicia o servidor de sockets
    console.log("[Node] Iniciando o ServidorSocket.exe...");
    servidorProcess = spawn(paths.servidor, [], {
      cwd: path.dirname(paths.servidor),
      detached: false,
    });

    // Captura saída do servidor
    servidorProcess.stdout.on("data", (data) => {
      console.log(`[Servidor ${tipo}]: ${data}`);
    });

    servidorProcess.stderr.on("data", (data) => {
      console.error(`[Servidor ${tipo} ERRO]: ${data}`);
    });

    // Delay de 2s para garantir que o servidor está pronto antes de iniciar o cliente
    setTimeout(() => {
      console.log(`[Node] Iniciando o Cliente ${tipo}...`);
        
      // Inicia o cliente de sockets
      clienteProcess = spawn(paths.cliente, [], {
        cwd: path.dirname(paths.cliente),
        detached: false,
      });

      // Captura saída do cliente
      clienteProcess.stdout.on("data", (data) => {
        console.log(`[Cliente ${tipo}]: ${data}`);
      });

      clienteProcess.stderr.on("data", (data) => {
        console.error(`[Cliente ${tipo} ERRO]: ${data}`);
      });

      // Conecta Node.js ao servidor C++ via socket TCP
      client = new net.Socket();
      client.connect(54000, "127.0.0.1", () => {
        console.log("[Node] Conectado ao servidor C++!");
      });

      let buffer = ""; // buffer para acumular dados recebidos
      client.on("data", (data) => {
        buffer += data.toString();
        let index;
        // Divide o buffer por linhas completas
        while ((index = buffer.indexOf("\n")) >= 0) {
          const linha = buffer.substring(0, index).trim();
          buffer = buffer.substring(index + 1);

          if (linha.length > 0) {
            try {
              // Tenta interpretar a linha como JSON
              const msg = JSON.parse(linha);
              mensagens.push(msg); // adiciona ao histórico
              console.log("[Node] Recebido do C++:", msg);
            }
            catch (e) {
              console.error("[Node] JSON inválido:", linha);
            }
          }
        }
      });

      // Captura erros na conexão socket
      client.on("error", (err) => {
        console.error("[Node] Erro no socket:", err.message);
      });

      // Evento disparado quando socket fecha
      client.on("close", () => {
        console.log("[Node] Conexão com servidor C++ fechada");
      });
    }, 2000);
  }

  //TO DO:
  // -------------------------
  // Caso 3: MEMORIA COMPARTILHADA
  // -------------------------
  else if (tipo === 'sharedmemory') {
    mensagens = []; // limpa histórico

    console.log("[Node] Iniciando Processo A e Processo B de Shared Memory...");

    // Inicia processo servidor (A)
    servidorProcess = spawn(paths.servidor, [], {
      cwd: path.dirname(paths.servidor),
      detached: false,
    });
    // Inicia processo cliente (B)
    clienteProcess = spawn(paths.cliente, [], {
      cwd: path.dirname(paths.cliente),
      detached: false,
    });

    // Captura saída do Processo A
    servidorProcess.stdout.on("data", (data) => {
      const linhas = data.toString().split("\n");
      linhas.forEach(linha => {
        if (linha.trim().length > 0) {
          try {
            const msg = JSON.parse(linha.trim()); // Tenta interpretar a linha como JSON
            mensagens.push(msg); // adiciona ao histórico
          } catch (e) {
            console.error("[Node] JSON inválido do shared memory:", linha);
          }

          console.log("[Node] [ProcA]:", linha.trim());
        }
      });
    });

    // Captura saída do Processo B
    clienteProcess.stdout.on("data", (data) => {
      const linhas = data.toString().split("\n");
      linhas.forEach(linha => {
        if (linha.trim().length > 0) {
          try {
            const msg = JSON.parse(linha.trim()); // Tenta interpretar a linha como JSON
            mensagens.push(msg);  // adiciona ao histórico
          } catch (e) {
            console.error("[Node] JSON inválido do shared memory:", linha);
          }

          console.log("[Node] [ProcA]:", linha.trim());
        }
      });
    });
  }

  // Responde ao front-end informando que determinado processo foi iniciado
  res.json({ status: `Processos de ${tipo} iniciados!` });
});

// Evento que vai ser disparado quando um processo filho é encerrado
if (servidorProcess) {
    servidorProcess.on('exit', (code, signal) => {
        console.log(`[Node] Processo servidor (PID: ${servidorProcess.pid}) encerrado.`);
        servidorProcess = null;
    });
}
// Evento disparado quando o processo cliente termina
if (clienteProcess) {
    clienteProcess.on('exit', (code, signal) => {
        console.log(`[Node] Processo cliente (PID: ${clienteProcess.pid}) encerrado.`);
        clienteProcess = null;
    });
}

// Endpoint para parar o processo atual
app.get("/stop-processo/:tipo", (req, res) => {
  // Verifica se existe algum processo rodando
  if (!servidorProcess && !clienteProcess) {
      return res.json({ status: "Nenhum processo está rodando." });
  }

  try {
    const tipoParado = processoAtual; // Captura o tipo que será finalizado antes de resetar
    if (clienteProcess && clienteProcess.pid) {
      // Mata o processo cliente via PID
      exec(`taskkill /F /PID ${clienteProcess.pid}`);
    }
      clienteProcess = null;
    
    if (servidorProcess && servidorProcess.pid) {
      // Mata o processo cliente via PID
      exec(`taskkill /F /PID ${servidorProcess.pid}`);
    }
      servidorProcess = null;
    
    if (client) {
      // Fecha socket caso esteja aberto
      client.destroy();
      client = null;
    }
    mensagens = []; // limpa histórico ao parar
    console.log("[Node] Histórico de mensagens limpo ao parar processo");
    
    // Reseta a variável de controle de tipo de processo
    processoAtual = null;
    
    // Retorna pro front-end o tipo de processo que foi finalizado
    res.json({ status: `Processos de ${tipoParado} finalizados e histórico limpo!` });

  } catch (err) {
    console.error("Erro ao finalizar:", err);
    res.status(500).json({ status: "Erro ao finalizar processos" });
  }
});

// O Front-end manda mensagem -> Cliente C++. Endpoint POST para enviar mensagem para o cliente C++
app.post("/enviar-mensagem", (req, res) => {
  const mensagem = req.body.mensagem; // Recebe o objeto com a mensagem do front-end

  if (clienteProcess && clienteProcess.stdin.writable) {
    try {
      // Envia mensagem via stdin do processo cliente
      clienteProcess.stdin.write(mensagem + "\n");
      res.json({ status: "Mensagem enviada com sucesso." });
    } catch (err) {
      res.status(500).json({ status: "Erro ao enviar mensagem para o cliente C++." });
    }
  } else {
    res.status(400).json({ status: "Processo do cliente não está rodando." });
  }
});

// Endpoint POST para enviar mensagens para processos de Shared Memory
app.post("/enviar-mensagem-sm", (req, res) => {
// Recebe o objeto com as duas mensagens do front-end
  const { mensagemA, mensagemB } = req.body;

  if (!servidorProcess || !clienteProcess) {
    return res.status(400).json({ status: "Processos de Shared Memory não estão rodando." });
  }

  try {
    // Envia mensagem via stdin do processo A
    if (mensagemA && servidorProcess.stdin.writable) {
      servidorProcess.stdin.write(mensagemA + "\n");
    }
    // Envia mensagem via stdin do processo B
    if (mensagemB && clienteProcess.stdin.writable) {
      clienteProcess.stdin.write(mensagemB + "\n");
    }
    res.json({ status: "Mensagens enviadas para os processos de Shared Memory." });
  } catch (err) {
    console.error("Erro ao enviar mensagens:", err);
    res.status(500).json({ status: "Erro ao enviar mensagens." });
  } 
});

// Endpoint POST para limpar histórico de mensagens manualmente
app.post("/limpar-mensagens", (req, res) => {
  console.log("[Node] Limpando o histórico de mensagens...");
  mensagens = []; // Esvazia o array de mensagens
  res.json({ status: "Histórico de mensagens limpo!" });
});

// Inicia o servidor HTTP na porta 3000
app.listen(3000, () => {
  console.log("[Node] HTTP em http://localhost:3000");
});
