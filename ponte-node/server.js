// const express = require("express");
// const net = require("net");
// const cors = require("cors");
// const { spawn } = require("child_process");
// const path = require('path'); // Mantém o path para o cwd

// const app = express();
// app.use(cors());
// app.use(express.json());

// let ultimaMensagem = { processo: 0, mensagem: "Nenhuma mensagem recebida ainda." };

// let servidorProcess = null;
// let clienteProcess = null;

// // Variável para a conexão do socket Node com o servidor C++
// let client = null;

// // Endpoint HTTP que o front-end chama
// app.get("/mensagens", (req, res) => {
//   res.json(ultimaMensagem);
// });

// // Iniciar os .exe
// app.get("/start-sockets", (req, res) => {
//   if (servidorProcess || clienteProcess) {
//     return res.json({ status: "Já está rodando!" });
//   }

//   console.log("[Node] Iniciando o ServidorSocket.exe...");

//   // Use o caminho absoluto para o executável
//   const serverExePath = 'C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ServidorSocket.exe';
//   const clientExePath = 'C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ClienteSocket.exe';

//   servidorProcess = spawn(serverExePath, [], {
//     cwd: path.dirname(serverExePath),
//     detached: false
//   });

//   servidorProcess.stdout.on("data", (data) => {
//     console.log(`[ServidorSocket]: ${data}`);
//   });

//   servidorProcess.stderr.on("data", (data) => {
//     console.error(`[ServidorSocket ERRO]: ${data}`);
//   });

//   // Aguarda o servidor C++ estar ativo antes de abrir o cliente C++
//   setTimeout(() => {
//     console.log("[Node] Iniciando o ClienteSocket.exe...");
//     
//     clienteProcess = spawn(clientExePath, [], {
//       cwd: path.dirname(clientExePath),
//       detached: false
//     });

//     clienteProcess.stdout.on("data", (data) => {
//       console.log(`[ClienteSocket]: ${data}`);
//     });

//     clienteProcess.stderr.on("data", (data) => {
//       console.error(`[ClienteSocket ERRO]: ${data}`);
//     });

//     // Conecta o Node ao Servidor C++ APENAS após ambos estarem rodando
//     client = new net.Socket();
//     client.connect(54000, "127.0.0.1", () => {
//       console.log("[Node] Conectado ao servidor C++!");
//     });

//     let buffer = "";
//     client.on("data", (data) => {
//       buffer += data.toString();
//       let index;
//       while ((index = buffer.indexOf("\n")) >= 0) {
//         const linha = buffer.substring(0, index);
//         buffer = buffer.substring(index + 1);

//         try {
//           ultimaMensagem = JSON.parse(linha);
//           console.log("[Node] Recebido do C++:", ultimaMensagem);
//         } catch (e) {
//           console.error("[Node] JSON inválido:", linha);
//         }
//       }
//     });

//     client.on("error", (err) => {
//       console.error("[Node] Erro no socket:", err.message);
//     });

//     client.on("close", () => {
//       console.log("[Node] Conexão com servidor C++ fechada");
//     });

//   }, 2000); // 2 segundos de espera

//   res.json({ status: "Sockets iniciados!" });
// });

// // Apenas no endpoint '/stop-sockets' do seu server.js
// app.get("/stop-sockets", (req, res) => {
//   try {
//     if (clienteProcess && clienteProcess.pid) { // Verifica se o processo existe e tem um PID
//       process.kill(-clienteProcess.pid);
//       clienteProcess = null;
//     }
//     if (servidorProcess && servidorProcess.pid) {
//       process.kill(-servidorProcess.pid);
//       servidorProcess = null;
//     }
//     if (client) {
//       client.destroy();
//       client = null;
//     }
//     res.json({ status: "Sockets finalizados!" });
//   } catch (err) {
//     console.error("Erro ao finalizar:", err);
//     res.status(500).json({ status: "Erro ao finalizar processos" });
//   }
// });

// // Endpoint para receber a mensagem do frontend e enviar para o Cliente C++
// app.post("/enviar-mensagem", (req, res) => {
//   const mensagem = req.body.mensagem;

//   if (clienteProcess && clienteProcess.stdin.writable) {
//     try {
//       clienteProcess.stdin.write(mensagem + "\n");
//       res.json({ status: "Mensagem enviada com sucesso." });
//     } catch (err) {
//       res.status(500).json({ status: "Erro ao enviar mensagem para o cliente C++." });
//     }
//   } else {
//     res.status(400).json({ status: "Processo do cliente não está rodando." });
//   }
// });

// // Sobe servidor HTTP
// app.listen(3000, () => {
//   console.log("[Node] HTTP em http://localhost:3000");
// });

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

// Endpoint: o front vai buscar a lista completa de mensagens
app.get("/mensagens", (req, res) => {
  res.json(mensagens);
});

// Iniciar os .exe
app.get("/start-sockets", (req, res) => {
  if (servidorProcess || clienteProcess) {
    return res.json({ status: "Já está rodando!" });
  }

  console.log("[Node] Iniciando o ServidorSocket.exe...");

  const serverExePath = "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ServidorSocket.exe";
  const clientExePath = "C:/Users/Usuario/Documents/ProjetoRA1-GP10/Backend/x64/Debug/ClienteSocket.exe";

  servidorProcess = spawn(serverExePath, [], {
    cwd: path.dirname(serverExePath),
    detached: false,
  });

  servidorProcess.stdout.on("data", (data) => {
    console.log(`[ServidorSocket]: ${data}`);
  });

  servidorProcess.stderr.on("data", (data) => {
    console.error(`[ServidorSocket ERRO]: ${data}`);
  });

  // espera o servidor C++ ligar antes do cliente
  setTimeout(() => {
    console.log("[Node] Iniciando o ClienteSocket.exe...");

    clienteProcess = spawn(clientExePath, [], {
      cwd: path.dirname(clientExePath),
      detached: false,
    });

    clienteProcess.stdout.on("data", (data) => {
      console.log(`[ClienteSocket]: ${data}`);
    });

    clienteProcess.stderr.on("data", (data) => {
      console.error(`[ClienteSocket ERRO]: ${data}`);
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

            if (linha.length > 0) { // Verifica se a linha não está vazia
                try {
                const msg = JSON.parse(linha);
                mensagens.push(msg); // adiciona no histórico
                console.log("[Node] Recebido do C++:", msg);
                } catch (e) {
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

  res.json({ status: "Sockets iniciados!" });
});

// parar processos
app.get("/stop-sockets", (req, res) => {
  try {
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
    res.json({ status: "Sockets finalizados!" });
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
