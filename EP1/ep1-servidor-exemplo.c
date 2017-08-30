/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 9/8/2017
 *
 * Um codigo simples (nao eh o codigo ideal, mas eh o suficiente para o  EP)  de
 * um servidor de eco a ser usado como base para o EP1. Ele recebe uma linha  de
 * um cliente e devolve a mesma linha. Teste ele assim depois de compilar:
 *
 * ./servidor 8000
 *
 * Com este comando o servidor ficara escutando por conexoes na porta  8000  TCP
 * (Se voce quiser fazer o servidor escutar em uma porta  menor  que  1024  voce
 * precisa ser root).
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 *
 * telnet 127.0.0.1 8000
 *
 * Escreva sequencias de caracteres seguidas de ENTER. Voce vera  que  o  telnet
 * exibe a mesma linha em seguida. Esta  repeticao  da  linha  eh  enviada  pelo
 * servidor. O servidor tambem exibe no terminal onde  ele  estiver  rodando  as
 * linhas enviadas pelos clientes.
 *
 * Obs.: Voce pode conectar  no  servidor  remotamente  tambem.  Basta  saber  o
 * endereço IP remoto da maquina onde o servidor esta rodando e nao  pode  haver
 * nenhum firewall no meio do caminho bloqueando conexoes na porta escolhida.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv)
{
    // Os sockets. Um que sera o socket que vai escutar pelas conexoes e o outro
    // que vai ser o socket especifico de cada conexao
    int listenfd, connfd;

    // Informacos sobre o socket (endereço e porta) ficam nesta struct
    struct sockaddr_in servaddr;

    // Retorno da funcao fork para saber quem eh o processo filho e  quem  eh  o
    // processo pai
    pid_t childpid;

    // Armazena linhas recebidas do cliente
	char recvline[MAXLINE + 1];

    // Armazena o tamanho da string lida do cliente
    ssize_t n;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <Porta>\n", argv[0]);
        fprintf(stderr, "Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    // Criacao de um socket. Eh como  se  fosse  um  descritor  de  arquivo.  Eh
    // possivel fazer operacoes como read, write e close. Neste  caso  o  socket
    // criado eh um socket IPv4 (por causa do AF_INET), que vai  usar  TCP  (por
    // causa do SOCK_STREAM), ja que o IMAP funciona sobre  TCP,  e  sera  usado
    // para uma aplicacao convencional sobre a Internet (por causa do número 0)
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket :(\n");
		exit(2);
	}

    // Agora eh necessario informar os enderecos associados a  este  socket.  Eh
    // necessario informar o endereco / interface e a porta, pois mais adiante o
    // socket ficara esperando conexoes nesta porta e neste(s)  enderecos.  Para
    // isso eh necessario preencher a struct servaddr. Eh necessario colocar  la
    // o tipo de socket (No nosso caso AF_INET porque eh IPv4), em qual endereco
    // interface serao  esperadas  conexoes  (Neste  caso  em  qualquer  uma  --
    // INADDR_ANY) e qual a porta. Neste caso sera a porta que foi passada  como
    // argumento no shell (atoi(argv[1]))
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    // Como este codigo eh o codigo de um servidor,  o  socket  sera  um  socket
    // passivo. Para isto eh necessario chamar a funcao listen  que  define  que
    // este eh um socket de servidor  que  ficara  esperando  por  conexoes  nos
    // enderecos definidos na funcao bind
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexoes na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    // O servidor no final das contas eh um loop infinito de espera por conexoes
    // processamento de cada uma individualmente
    for (;;) {
        // O socket inicial que foi criado eh o socket  que  vai  aguardar  pela
        // conexao na porta especificada. Mas  pode  ser  que  existam  diversos
        // clientes conectando no servidor. Por isso deve-se utilizar  a  funcao
        // accept. Esta funcao vai retirar uma conexao da fila de  conexoes  que
        // foram aceitas no socket listenfd e vai  criar  um  socket  especifico
        // para esta conexao. O descritor deste novo  socket  eh  o  retorno  da
        // funcao accept
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }

        // Agora o servidor precisa tratar este cliente de forma separada.  Para
        // isto eh criado um processo filho usando a funcao fork. O processo vai
        // ser uma copia deste. Depois da funcao fork, os dois processos (pai  e
        // filho) estarao no mesmo ponto do codigo, mas  cada  um  tera  um  PID
        // diferente. Assim eh possivel diferenciar o que cada processo tera que
        // fazer. O filho tem que processar a requisicao do cliente. O  pai  tem
        // que voltar no loop para continuar aceitando novas conexões

        // Se o retorno da funcao fork for zero, eh porque esta no processo filho.
        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            printf("[Uma conexao aberta]\n");
            // Ja que esta  no  processo  filho,  nao  precisa  mais  do  socket
            // listenfd. So o processo pai precisa deste socket
            close(listenfd);

            // Agora pode ler do socket e escrever no socket. Isto tem  que  ser
            // feito em sincronia com o cliente. Nao faz sentido ler sem  ter  o
            // que ler. Ou seja, neste caso esta sendo considerado que o cliente
            // vai enviar algo para o servidor. O servidor vai processar  o  que
            // tiver sido enviado e vai enviar uma resposta para o cliente  (Que
            // precisara estar esperando por esta resposta)

            //================================================================//
            //                           EP1 INICIO                           //
            //================================================================//
            // TODO: Eh esta parte do codigo que tera que  ser  modificada  para
            // que este servidor consiga interpretar comandos IMAP
            while ((n = read(connfd, recvline, MAXLINE)) > 0) {
                recvline[n] = 0;
                printf("[Cliente conectado no processo filho %d enviou:] ", getpid());
                if ((fputs(recvline, stdout)) == EOF) {
                    perror("fputs :(\n");
                    exit(6);
                }
                write(connfd, recvline, strlen(recvline));
            }
            //================================================================//
            //                            EP1 FIM                             //
            //================================================================//

            // Apos ter feito toda a troca de informacao  com  o  cliente,  pode
            // finalizar o processo filho
            printf("[Uma conexao fechada]\n");
            exit(0);
        }
        /**** PROCESSO PAI ****/
        // Se for o pai, a unica coisa a ser feita eh  fechar  o  socket  connfd
        // (ele eh o socket do cliente especifico que sera tratado pelo processo
        // filho)
        close(connfd);
    }
    exit(0);
}
