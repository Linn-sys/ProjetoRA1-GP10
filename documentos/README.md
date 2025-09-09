# ProjetoRA1-GP10
### Repositório para o desenvolvimento do projeto RA1 da matéria de sistemas de computação PUCPR

## Pipes
### Essencialmente implementei um **pipe** que com sucesso transmite uma string digitada pelo usuário do processo principal para o filho e então imprime no filho, é necessário implementar JSON para controle de fluxo e integração com o FRONTEND. O PipeMain é o processo que cria o processo filho chamado aptamente de Offshoot para enviar dados por pipe anônimo utilizando a api do windows.

## Sockets
### Têm-se um **Servidor Socket** (arquivo c++) que é iniciado na porta 54000 e um **Cliente Socket** (arquivo c++) que se conecta nessa mesma porta, e quando o cliente envia uma mensagem para o servidor, essa troca de informações é acompanhada e registrada pelo servidor node, que recupera as mensagens e envia para o front, mostrando na tela a mensagem *X* enviada do cliente, e o aviso de que o servidor recebeu *X* mensagem.

## Memória Compartilhada
### TO DO

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
