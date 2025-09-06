async function carregarMensagens() {
    try {
        const res = await fetch("http://localhost:3000/mensagens");
        const dados = await res.json();

        const proc1 = document.querySelector(".section-process_1_content");
        const proc2 = document.querySelector(".section-process_2_content");

        // Cria log sem apagar o anterior
        if (dados && dados.mensagem) {
            const span = document.createElement("span");
            span.classList.add("text", "subtitle");
            span.innerText = dados.mensagem;

            if (dados.processo === 1) {
                proc1.appendChild(span);
                proc1.scrollTop = proc1.scrollHeight; // scroll automático
            } else if (dados.processo === 2) {
                proc2.appendChild(span);
                proc2.scrollTop = proc2.scrollHeight; // scroll automático
            }
        }
    } catch (err) {
        console.error("Erro ao carregar mensagens:", err);
    }
}

// Atualiza a cada 1.5s
setInterval(carregarMensagens, 1500);
carregarMensagens();
