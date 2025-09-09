# ProjetoRA1-GP10
### Repositório para o desenvolvimento do projeto RA1 da matéria de sistemas de computação PUCPR

## Observações
### O backend do projeto inteiro foi desenvolvido em C++, já o frontend foi desenvolvido em node.js, html e CSS.

## Pipes
### Essencialmente foi implementado um **Pipe** que com sucesso transmite uma mensagem digitada pelo usuário do processo principal para o filho e então imprime no filho, JSON será usado para monitorar o fluxo de dados. O PipeMain é o processo que cria o processo filho chamado aptamente de Offshoot para enviar dados por pipe anônimo utilizando a api do windows.

## Sockets
### Têm-se um **Servidor Socket** (arquivo c++) que é iniciado na porta 54000 e um **Cliente Socket** (arquivo c++) que se conecta nessa mesma porta, e quando o cliente envia uma mensagem para o servidor, essa troca de informações é acompanhada e registrada pelo servidor node, que recupera as mensagens e envia para o front, mostrando na tela a mensagem *X* enviada do cliente, e o aviso de que o servidor recebeu *X* mensagem.

## Memória Compartilhada
### Possuí 2 processos e um buffer. Cada processo consegue ler e escrever no Buffer. Os dois processos são os mesmos, e conseguem utilizar o mesmo Buffer compartilhado e o mesmo Mutex para sincronização. Possuem 2 threads que ficam rodando simultaneamente em ambos os processos, uma de leitura do buffer (analisa se houve alteração), e outra de input do usuário. Ao digitar 0, é encerrado o programa em ambos os processos, além das threads. 

## Testes Realizados
### Testamos a possibilidade de enviar dados dos 2 processos na memória compartilhada e o envio é bidirecional com sucesso, a funcionalidade de encerramentos de todos os 3 tipos de programa e todos encerram adequadamente, a formatação JSON do estado do programa funciona totalmente.

## Instruções para baixar o Git (caso ainda não o tenha instalado na máquina)
### Faça o download do instalador executável no site: [Clique Aqui!](https://git-scm.com/downloads/win). Após baixar, abra o instalador e clique Next até o fim. 
### Pontos importantes:
### - Editor padrão: pode deixar “Vim” ou mudar pra “Notepad” se preferir.
### - PATH environment: escolha “Git from the command line” (opção padrão).
### - Line endings: deixa em “Checkout Windows-style, commit Unix-style” (padrão).
### - O resto pode deixar default.
### Finaliza com Install e depois Finish.
### Para verificar se a instalação ocorreu com sucesso, abra o Prompt de Comando e escreva: 
#### git --version
### Se a versão aparecer instalada, então instalou certinho.
### Além disso, efetue no CMD: 
#### git config --global user.name "Seu Nome"
#### git config --global user.email "seuemail@exemplo.com"

## Instruções para utilizar o Node (Passo 3 do Guia)
### Faça o download do Node.js (versão LTS) no site: [Clique Aqui!](https://nodejs.org/pt). Clique em Next em todas as etapas e instale. Para verificar se a instalação ocorreu com sucesso, abra o Prompt de Comando e escreva: 
#### node -v
#### npm -v
### Se as versões aparecerem instaladas, então tudo ocorreu como o esperado.

## Instruções de Compilação
### Abra o arquivo backend.sln no visual studio, o que vai abrir a solução backend, depois compile todos os projetos como preferir desde que todos sejam compilados e possuam um exe correspondente, depois siga as instruções a seguir de execução do projeto.

## Passos para executar corretamente o projeto:

Passos    |  Descrição
:-------- | :-----------------------------------------------------------------------------------------------------
Passo 1   | Abrir a pasta (ProjetoRA1-GP10\ponte-node) no Windows Explorer
Passo 2   | Clique no path dessa pasta e escreva cmd. Após isso, aperte a tecla Enter.
Passo 3   | No terminal que irá abrir, escreva 'node server.js' (sem as aspas) e aperta a tecla Enter.
Passo 4   | Abra a pasta (ProjetoRA1-GP10\Frontend) no Windows Explorer, e abra o arquivo index.html via Google Chrome.
Passo 5   | Selecione o tipo de processo nas opções. (Todos os arquivos já estão compilados com seus devidos .exe criados)
Passo 6   | Clique no botão 'Iniciar Processo'.
Passo 7   | Na seção "Informe uma mensagem de entrada" envie uma mensagem no input e aperte o botão 'Enviar'.
Passo 8   | No caso da opção ser Pipes: no log do Remetente aparecerá que ele enviou uma mensagem. No log do Destinatário aparecerá a mensagem recebida do Remetente.
Passo 8.1 | No caso da opção ser Socket: A mensagem enviada aparecerá no log do Cliente e o log do Servidor dirá que recebeu e processou determinada mensagem.
Passo 8.2 | No caso da opção ser Memória Compartilhada: O usuário poderá enviar mensagens por duas vias diferentes, o Processo A (ProcA) e o Processo B (ProcB), que compartilham do mesmo espaço de memória. Um dos campos necessariamente precisa conter algum texto, não é possível enviar duas mensagens vazias, nulas. Ao enviar uma mensagem pelo ProcA, o log retornará dizendo que o ProcA alterou o buffer e exibirá o texto atualmente alocado no buffer, enquanto no log do ProcB será exibido somente a nova mensagem. O inverso se aplica, se escrever no ProcB, o log dele exibirá que o ProcB alterou o buffer, e mostrará o texto atual, enquanto no log do ProcA somente será mostrado o texto atual.
Passo 8.2.1 | Para encerrar a Memória Compartilhada, basta clicar no botão 'Finalizar Processo' ou digitar em qualquer um dos inputs '0', clicar no botão 'Enviar', encerrando os chats entre os dois processos, e depois  clicar no botão 'Finalizar Processo'.
Passo 9     | Quando quiser sair totalmente do projeto, basta fechar a página index.html e no terminal apertar CTRL + C, que vai encerrar o Node. 
