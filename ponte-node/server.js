const express = require("express");
const net = require("net");
const cors = require("cors");

const app = express();
app.use(cors());

let ultimaMensagem = { processo: 0, mensagem: "Nenhuma mensagem recebida ainda." };

// Conexão TCP com servidor C++
const client = new net.Socket();
client.connect(54000, "127.0.0.1", () => {
  console.log("[Node] Conectado ao servidor C++!");
});

let buffer = "";

client.on("data", (data) => {
  buffer += data.toString();
  let index;
  while ((index = buffer.indexOf("\n")) >= 0) {
    const linha = buffer.substring(0, index);
    buffer = buffer.substring(index + 1);

    try {
      ultimaMensagem = JSON.parse(linha);
      console.log("[Node] Recebido do C++:", ultimaMensagem);
    } catch (e) {
      console.error("[Node] JSON inválido:", linha);
    }
  }
});


client.on("error", (err) => {
  console.error("[Node] Erro no socket:", err.message);
});

client.on("close", () => {
  console.log("[Node] Conexão com servidor C++ fechada");
});

// Endpoint HTTP que o front-end chama
app.get("/mensagens", (req, res) => {
  res.json(ultimaMensagem);
});

// Sobe servidor HTTP
app.listen(3000, () => {
  console.log("[Node] HTTP em http://localhost:3000");
});
