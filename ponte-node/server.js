const express = require("express");
const net = require("net");
const cors = require("cors");
const { spawn, exec } = require("child_process");
const path = require("path");

const app = express();
app.use(cors());
app.use(express.json());

// agora guardamos TODAS as mensagens
let mensagens = [];
let servidorProcess = null;
let clienteProcess = null;
let client = null; // conexão Node -> Servidor C++
let processoAtual = null;

// Mapa para guardar os caminhos dos executáveis
const executaveis = {
  sockets: {
    servidor: "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ServidorSocket.exe",
    cliente: "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ClienteSocket.exe",
  },
  pipes: {
    servidor: "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/PipeOffshoot.exe",
    cliente: "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/PipeMain.exe",
  },
  sharedmemory: {
    servidor: null,
    cliente: null,
  }
};

// Endpoint: o front vai buscar a lista completa de mensagens
app.get("/mensagens", (req, res) => {
  res.json(mensagens);
});

// Iniciar os .exe
// NOVO: Endpoint genérico para iniciar um processo
app.get("/start-processo/:tipo", (req, res) => {
  const tipo = req.params.tipo;
  const paths = executaveis[tipo];
  
  if (!paths || !paths.servidor || !paths.cliente) {
    return res.status(400).json({ status: "Opção de processo inválida ou não implementada." });
  }

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
    mensagens = [];
    // Inicia o cliente (PipeMain), que por sua vez cria o destinatário
    console.log("[Node] Iniciando PipeMain (que cria o Offshoot)...");

    clienteProcess = spawn(paths.cliente, [], {
      cwd: path.dirname(paths.cliente),
      detached: false,
    });

    let bufferPipe = "";

    // Escuta mensagens do PipeMain (remetente)
    clienteProcess.stdout.on("data", (data) => {
      bufferPipe += data.toString();
      let index;
      while ((index = bufferPipe.indexOf("\n")) >= 0) {
        const linha = bufferPipe.substring(0, index).trim();
        bufferPipe = bufferPipe.substring(index + 1);

        if (linha.length > 0) {
          try {
            const msg = JSON.parse(linha);
            mensagens.push(msg);
            console.log("[Node] Recebido do PipeMain:", msg);
          } catch (e) {
            console.error("[Node] JSON inválido do PipeMain:", linha);
          }
        }
      }
    });

    clienteProcess.stderr.on("data", (data) => {
      console.error(`[PipeMain ERRO]: ${data}`);
    });
    
  }

  // -------------------------
  // Caso 2: SOCKETS
  // -------------------------
  else if (tipo === 'sockets') {
    mensagens = [];
    // Inicia o Servidor C++
    console.log("[Node] Iniciando o ServidorSocket.exe...");
    servidorProcess = spawn(paths.servidor, [], {
      cwd: path.dirname(paths.servidor),
      detached: false,
    });

    servidorProcess.stdout.on("data", (data) => {
      console.log(`[Servidor ${tipo}]: ${data}`);
    });

    servidorProcess.stderr.on("data", (data) => {
      console.error(`[Servidor ${tipo} ERRO]: ${data}`);
    });

    setTimeout(() => {
      console.log(`[Node] Iniciando o Cliente ${tipo}...`);
        
      clienteProcess = spawn(paths.cliente, [], {
        cwd: path.dirname(paths.cliente),
        detached: false,
      });

      clienteProcess.stdout.on("data", (data) => {
        console.log(`[Cliente ${tipo}]: ${data}`);
      });

      clienteProcess.stderr.on("data", (data) => {
        console.error(`[Cliente ${tipo} ERRO]: ${data}`);
      });

      // conecta Node -> Servidor C++
      client = new net.Socket();
      client.connect(54000, "127.0.0.1", () => {
        console.log("[Node] Conectado ao servidor C++!");
      });

      let buffer = "";
      client.on("data", (data) => {
        buffer += data.toString();
        let index;
        while ((index = buffer.indexOf("\n")) >= 0) {
          const linha = buffer.substring(0, index).trim();
          buffer = buffer.substring(index + 1);

          if (linha.length > 0) {
            try {
              const msg = JSON.parse(linha);
              mensagens.push(msg);
              console.log("[Node] Recebido do C++:", msg);
            }
            catch (e) {
              console.error("[Node] JSON inválido:", linha);
            }
          }
        }
      });

      client.on("error", (err) => {
        console.error("[Node] Erro no socket:", err.message);
      });

      client.on("close", () => {
        console.log("[Node] Conexão com servidor C++ fechada");
      });
    }, 2000);
  }

  //TO DO:
  // -------------------------
  // Caso 3: MEMORIA COMPARTILHADA
  // -------------------------
  // else if (tipo === 'sharedmemory') {
  //   mensagens = [];
  //   // Inicia o cliente (PipeMain), que por sua vez cria o destinatário
  //   console.log("[Node] Iniciando PipeMain (que cria o Offshoot)...");

  //   clienteProcess = spawn(paths.cliente, [], {
  //     cwd: path.dirname(paths.cliente),
  //     detached: false,
  //   });

  //   let bufferPipe = "";

  //   // Escuta mensagens do PipeMain (remetente)
  //   clienteProcess.stdout.on("data", (data) => {
  //     bufferPipe += data.toString();
  //     let index;
  //     while ((index = bufferPipe.indexOf("\n")) >= 0) {
  //       const linha = bufferPipe.substring(0, index).trim();
  //       bufferPipe = bufferPipe.substring(index + 1);

  //       if (linha.length > 0) {
  //         try {
  //           const msg = JSON.parse(linha);
  //           mensagens.push(msg);
  //           console.log("[Node] Recebido do PipeMain:", msg);
  //         } catch (e) {
  //           console.error("[Node] JSON inválido do PipeMain:", linha);
  //         }
  //       }
  //     }
  //   });

  //   clienteProcess.stderr.on("data", (data) => {
  //     console.error(`[PipeMain ERRO]: ${data}`);
  //   });
    
  // }

  res.json({ status: `Processos de ${tipo} iniciados!` });
});

// NOVO: Endpoint genérico para parar um processo
app.get("/stop-processo/:tipo", (req, res) => {
  // A rota de stop não precisa do tipo, pois ela vai finalizar o que estiver ativo
    if (!servidorProcess && !clienteProcess) {
        return res.json({ status: "Nenhum processo está rodando." });
    }

  try {
    const tipoParado = processoAtual; // Captura o tipo que será finalizado
    if (clienteProcess && clienteProcess.pid) {
      exec(`taskkill /F /PID ${clienteProcess.pid}`, (err) => {
        if (err) console.error("Erro ao finalizar cliente:", err);
      });
      clienteProcess = null;
    }
    if (servidorProcess && servidorProcess.pid) {
      exec(`taskkill /F /PID ${servidorProcess.pid}`, (err) => {
        if (err) console.error("Erro ao finalizar servidor:", err);
      });
      servidorProcess = null;
    }
    if (client) {
      client.destroy();
      client = null;
    }
    mensagens = []; // limpa histórico ao parar
    console.log("[Node] Histórico de mensagens limpo ao parar processo");
    
    // Reseta a variável de controle
    processoAtual = null;
    
    // Retorna o tipo de processo que REALMENTE foi finalizado
    res.json({ status: `Processos de ${tipoParado} finalizados e histórico limpo!` });

  } catch (err) {
    console.error("Erro ao finalizar:", err);
    res.status(500).json({ status: "Erro ao finalizar processos" });
  }
});

// front manda mensagem -> Cliente C++
app.post("/enviar-mensagem", (req, res) => {
  const mensagem = req.body.mensagem;

  if (clienteProcess && clienteProcess.stdin.writable) {
    try {
      clienteProcess.stdin.write(mensagem + "\n");
      res.json({ status: "Mensagem enviada com sucesso." });
    } catch (err) {
      res.status(500).json({ status: "Erro ao enviar mensagem para o cliente C++." });
    }
  } else {
    res.status(400).json({ status: "Processo do cliente não está rodando." });
  }
});

// TO DO:
// app.post("/enviar-mensagem-sm", (req, res) => {
//     // Recebe o objeto com as duas mensagens do front-end
//     const { mensagemA, mensagemB } = req.body;

//     if (!servidorProcess) {
//         return res.status(400).json({ status: "Processos de Memória Compartilhada não estão rodando." });
//     }
    
//     // AQUI vai a lógica para enviar as mensagens para os executáveis C++ de Memória Compartilhada.

//     const clientSM = new net.Socket();
//     clientSM.connect(54001, "127.0.0.1", () => {
//         // Envia as mensagens em formato JSON para o serviço C++
//         clientSM.write(JSON.stringify({
//             procA: mensagemA,
//             procB: mensagemB
//         }));
//     });

//     clientSM.on("error", (err) => {
//         console.error("Erro ao conectar no serviço de Memória Compartilhada:", err);
//         res.status(500).json({ status: "Erro ao enviar mensagens para o C++." });
//     });

//     // Você pode capturar a resposta do serviço C++ aqui
//     clientSM.on("data", (data) => {
//         res.json({ status: data.toString() });
//         clientSM.destroy();
//     });
//     // Retorna uma resposta ao front-end
//     res.json({ status: "Mensagens de Memória Compartilhada enviadas para os processos C++." });
// });

// NOVO ENDPOINT para limpar as mensagens

app.post("/limpar-mensagens", (req, res) => {
  console.log("[Node] Limpando o histórico de mensagens...");
  mensagens = []; // Esvazia o array de mensagens
  res.json({ status: "Histórico de mensagens limpo!" });
});

// HTTP
app.listen(3000, () => {
  console.log("[Node] HTTP em http://localhost:3000");
});
