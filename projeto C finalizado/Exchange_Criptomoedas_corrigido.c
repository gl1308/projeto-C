
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

#define ARQ_USUARIO  "investidor.txt"
#define ARQ_COTACOES "cotacoes.txt"
#define ARQ_EXTRATO  "extrato.txt"

#define MAX_NOME 100
#define TAM_CPF 12
#define TAM_SENHA 7
#define MAX_VALOR 1000000000.0

typedef struct {
    char nome[MAX_NOME];
    char cpf[TAM_CPF];
    char senha[TAM_SENHA];
    double reais;
    double bitcoin;
    double ethereum;
    double ripple;
} Investidor;

typedef struct {
    double bitcoin;
    double ethereum;
    double ripple;
} Cotacoes;

static void descartarRestoDaLinha(void) {
    int caractere;
    while ((caractere = getchar()) != '\n' && caractere != EOF) {
    }
}

static int lerLinha(const char *mensagem, char *destino, size_t tamanho) {
    size_t comprimento;

    printf("%s", mensagem);
    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }

    comprimento = strlen(destino);
    if (comprimento > 0 && destino[comprimento - 1] == '\n') {
        destino[comprimento - 1] = '\0';
    } else if (!feof(stdin)) {
        descartarRestoDaLinha();
    }

    return 1;
}

static int lerInteiro(const char *mensagem, int *valor) {
    char linha[80];
    char *fim;
    long numero;

    if (!lerLinha(mensagem, linha, sizeof(linha)) || linha[0] == '\0') {
        return 0;
    }

    errno = 0;
    numero = strtol(linha, &fim, 10);
    while (isspace((unsigned char)*fim)) {
        fim++;
    }

    if (fim == linha || *fim != '\0' || errno == ERANGE ||
        numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

static int lerDouble(const char *mensagem, double *valor) {
    char linha[128];
    char *fim;
    double numero;
    size_t i;

    if (!lerLinha(mensagem, linha, sizeof(linha)) || linha[0] == '\0') {
        return 0;
    }


    for (i = 0; linha[i] != '\0'; i++) {
        if (linha[i] == ',') {
            linha[i] = '.';
        }
    }

    errno = 0;
    numero = strtod(linha, &fim);
    while (isspace((unsigned char)*fim)) {
        fim++;
    }

    if (fim == linha || *fim != '\0' || errno == ERANGE ||
        !isfinite(numero) || numero < 0.0 || numero > MAX_VALOR) {
        return 0;
    }

    *valor = numero;
    return 1;
}

static int cpfValido(const char *cpf) {
    int i;

    if (strlen(cpf) != 11) {
        return 0;
    }

    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) {
            return 0;
        }
    }
    return 1;
}

static int senhaValida(const char *senha) {
    int i;

    if (strlen(senha) != 6) {
        return 0;
    }

    for (i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)senha[i])) {
            return 0;
        }
    }
    return 1;
}

static double arredondarCentavos(double valor) {
    
    return (double)((long long)(valor * 100.0 + 0.5)) / 100.0;
}

static double arredondarOitoCasas(double valor) {
    if (valor > 90000000000.0) {
        return valor;
    }
    return (double)((long long)(valor * 100000000.0 + 0.5)) / 100000000.0;
}


static int criarArquivosIniciais(void) {
    FILE *arquivo;

    arquivo = fopen(ARQ_USUARIO, "r");
    if (arquivo == NULL) {
        arquivo = fopen(ARQ_USUARIO, "w");
        if (arquivo == NULL) {
            printf("Erro: nao foi possivel criar %s.\n", ARQ_USUARIO);
            return 0;
        }

        fprintf(arquivo,
                "Investidor Teste\n"
                "12345678900\n"
                "123456\n"
                "1000.00\n"
                "0.00000000\n"
                "0.00000000\n"
                "0.00000000\n");

        if (fclose(arquivo) != 0) {
            printf("Erro ao finalizar a gravacao de %s.\n", ARQ_USUARIO);
            return 0;
        }
        printf("Arquivo %s criado com um investidor de demonstracao.\n", ARQ_USUARIO);
    } else {
        fclose(arquivo);
    }

    arquivo = fopen(ARQ_COTACOES, "r");
    if (arquivo == NULL) {
        arquivo = fopen(ARQ_COTACOES, "w");
        if (arquivo == NULL) {
            printf("Erro: nao foi possivel criar %s.\n", ARQ_COTACOES);
            return 0;
        }

        fprintf(arquivo, "300000.00\n15000.00\n3.00\n");
        if (fclose(arquivo) != 0) {
            printf("Erro ao finalizar a gravacao de %s.\n", ARQ_COTACOES);
            return 0;
        }
    } else {
        fclose(arquivo);
    }

    arquivo = fopen(ARQ_EXTRATO, "a");
    if (arquivo == NULL) {
        printf("Erro: nao foi possivel criar/abrir %s.\n", ARQ_EXTRATO);
        return 0;
    }
    fclose(arquivo);

    return 1;
}

static int carregarInvestidor(Investidor *usuario) {
    FILE *arquivo = fopen(ARQ_USUARIO, "r");
    int lidos;

    if (arquivo == NULL) {
        return 0;
    }

    lidos = fscanf(arquivo, "%99[^\n]\n%11s\n%6s\n%lf\n%lf\n%lf\n%lf",
                   usuario->nome, usuario->cpf, usuario->senha,
                   &usuario->reais, &usuario->bitcoin,
                   &usuario->ethereum, &usuario->ripple);
    fclose(arquivo);

    if (lidos != 7 || usuario->nome[0] == '\0' ||
        !cpfValido(usuario->cpf) || !senhaValida(usuario->senha)) {
        return 0;
    }

    if (!isfinite(usuario->reais) || !isfinite(usuario->bitcoin) ||
        !isfinite(usuario->ethereum) || !isfinite(usuario->ripple) ||
        usuario->reais < 0.0 || usuario->bitcoin < 0.0 ||
        usuario->ethereum < 0.0 || usuario->ripple < 0.0) {
        return 0;
    }

    return 1;
}

static int salvarInvestidor(const Investidor *usuario) {
    FILE *arquivo = fopen(ARQ_USUARIO, "w");
    int ok = 1;

    if (arquivo == NULL) {
        printf("Erro: nao foi possivel salvar %s.\n", ARQ_USUARIO);
        return 0;
    }

    if (fprintf(arquivo, "%s\n%s\n%s\n%.2f\n%.8f\n%.8f\n%.8f\n",
                usuario->nome, usuario->cpf, usuario->senha,
                usuario->reais, usuario->bitcoin,
                usuario->ethereum, usuario->ripple) < 0) {
        ok = 0;
    }

    if (fclose(arquivo) != 0) {
        ok = 0;
    }

    if (!ok) {
        printf("Erro durante a gravacao dos dados do investidor.\n");
    }
    return ok;
}

static int carregarCotacoes(Cotacoes *cotacoes) {
    FILE *arquivo = fopen(ARQ_COTACOES, "r");
    int lidos;

    if (arquivo == NULL) {
        return 0;
    }

    lidos = fscanf(arquivo, "%lf%lf%lf", &cotacoes->bitcoin,
                   &cotacoes->ethereum, &cotacoes->ripple);
    fclose(arquivo);

    return lidos == 3 && isfinite(cotacoes->bitcoin) &&
           isfinite(cotacoes->ethereum) && isfinite(cotacoes->ripple) &&
           cotacoes->bitcoin > 0.0 && cotacoes->ethereum > 0.0 &&
           cotacoes->ripple > 0.0;
}

static int salvarCotacoes(const Cotacoes *cotacoes) {
    FILE *arquivo = fopen(ARQ_COTACOES, "w");
    int ok = 1;

    if (arquivo == NULL) {
        printf("Erro: nao foi possivel salvar %s.\n", ARQ_COTACOES);
        return 0;
    }

    if (fprintf(arquivo, "%.2f\n%.2f\n%.2f\n",
                cotacoes->bitcoin, cotacoes->ethereum,
                cotacoes->ripple) < 0) {
        ok = 0;
    }

    if (fclose(arquivo) != 0) {
        ok = 0;
    }

    if (!ok) {
        printf("Erro durante a gravacao das cotacoes.\n");
    }
    return ok;
}

static int registrarExtrato(const char *operacao, double valor,
                            const char *moeda, double cotacao,
                            double taxa, const Investidor *usuario) {
    FILE *arquivo = fopen(ARQ_EXTRATO, "a");
    time_t agora;
    struct tm *data;
    char dataHora[32] = "data indisponivel";
    int ok = 1;

    if (arquivo == NULL) {
        printf("Erro: nao foi possivel gravar o extrato.\n");
        return 0;
    }

    agora = time(NULL);
    data = localtime(&agora);
    if (data != NULL) {
        strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", data);
    }

    if (fprintf(arquivo,
                "%s | CPF: %s | %s | %.8f %s | cotacao: R$ %.2f | "
                "taxa: R$ %.2f | saldo R$: %.2f | BTC: %.8f | "
                "ETH: %.8f | XRP: %.8f\n",
                dataHora, usuario->cpf, operacao, valor, moeda, cotacao,
                taxa, usuario->reais, usuario->bitcoin,
                usuario->ethereum, usuario->ripple) < 0) {
        ok = 0;
    }

    if (fclose(arquivo) != 0) {
        ok = 0;
    }

    if (!ok) {
        printf("Erro ao concluir o registro da operacao no extrato.\n");
    }
    return ok;
}

/* Salva o saldo e registra o extrato. Se o extrato falhar, tenta reverter. */
static int confirmarOperacao(Investidor *usuario,
                             const Investidor *estadoAnterior,
                             const char *operacao, double valor,
                             const char *moeda, double cotacao,
                             double taxa) {
    if (!salvarInvestidor(usuario)) {
        *usuario = *estadoAnterior;
        return 0;
    }

    if (!registrarExtrato(operacao, valor, moeda, cotacao, taxa, usuario)) {
        *usuario = *estadoAnterior;
        if (!salvarInvestidor(usuario)) {
            printf("ATENCAO: nao foi possivel restaurar o saldo anterior. "
                   "Confira o arquivo %s.\n", ARQ_USUARIO);
        }
        return 0;
    }

    return 1;
}

/* ---------- Login e consultas ---------- */

static int autenticarSenha(const Investidor *usuario) {
    char senha[TAM_SENHA];

    if (!lerLinha("Confirme sua senha (6 digitos): ", senha, sizeof(senha))) {
        return 0;
    }

    if (!senhaValida(senha) || strcmp(senha, usuario->senha) != 0) {
        printf("Senha incorreta ou invalida.\n");
        return 0;
    }
    return 1;
}

static int login(const Investidor *usuario) {
    char cpf[TAM_CPF];
    char senha[TAM_SENHA];
    int tentativa;

    printf("\n=============== LOGIN ===============\n");

    for (tentativa = 1; tentativa <= 3; tentativa++) {
        if (!lerLinha("CPF (11 numeros, sem pontos ou tracos): ", cpf, sizeof(cpf)) ||
            !lerLinha("Senha numerica (6 digitos): ", senha, sizeof(senha))) {
            return 0;
        }

        if (cpfValido(cpf) && senhaValida(senha) &&
            strcmp(cpf, usuario->cpf) == 0 &&
            strcmp(senha, usuario->senha) == 0) {
            printf("\nLogin realizado com sucesso. Bem-vindo(a), %s!\n",
                   usuario->nome);
            return 1;
        }

        printf("CPF ou senha incorretos. Tentativa %d de 3.\n", tentativa);
    }

    printf("Numero maximo de tentativas atingido.\n");
    return 0;
}

static void consultarSaldo(const Investidor *usuario) {
    if (!autenticarSenha(usuario)) {
        return;
    }

    printf("\n========== SALDO DA CARTEIRA ==========\n");
    printf("Nome: %s\nCPF: %s\n", usuario->nome, usuario->cpf);
    printf("Reais:    R$ %.2f\n", usuario->reais);
    printf("Bitcoin:  %.8f BTC\n", usuario->bitcoin);
    printf("Ethereum: %.8f ETH\n", usuario->ethereum);
    printf("Ripple:   %.8f XRP\n", usuario->ripple);
}

static void consultarExtrato(const Investidor *usuario) {
    FILE *arquivo;
    char linha[512];
    int encontrou = 0;

    if (!autenticarSenha(usuario)) {
        return;
    }

    arquivo = fopen(ARQ_EXTRATO, "r");
    if (arquivo == NULL) {
        printf("Nao foi possivel abrir o extrato.\n");
        return;
    }

    printf("\n========== EXTRATO DE OPERACOES ==========\n");
    printf("Investidor: %s | CPF: %s\n\n", usuario->nome, usuario->cpf);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        /* O projeto individual possui um unico investidor. */
        if (strstr(linha, usuario->cpf) != NULL) {
            fputs(linha, stdout);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhuma operacao registrada.\n");
    }
    fclose(arquivo);
}

static void mostrarCotacoes(const Cotacoes *cotacoes) {
    printf("\n========== COTACOES ATUAIS (SIMULADAS) ==========\n");
    printf("1. Bitcoin  (BTC): R$ %.2f | Compra: 2%% | Venda: 3%%\n",
           cotacoes->bitcoin);
    printf("2. Ethereum (ETH): R$ %.2f | Compra: 1%% | Venda: 2%%\n",
           cotacoes->ethereum);
    printf("3. Ripple   (XRP): R$ %.2f | Compra: 1%% | Venda: 1%%\n",
           cotacoes->ripple);
}

/* ---------- Deposito e saque ---------- */

static void depositar(Investidor *usuario) {
    Investidor anterior = *usuario;
    double valor;

    if (!lerDouble("Valor do deposito em reais: R$ ", &valor) || valor <= 0.0) {
        printf("Valor invalido. Informe um numero maior que zero.\n");
        return;
    }

    valor = arredondarCentavos(valor);
    if (valor <= 0.0 || usuario->reais > MAX_VALOR - valor) {
        printf("Valor fora do limite permitido.\n");
        return;
    }

    usuario->reais = arredondarCentavos(usuario->reais + valor);
    if (!confirmarOperacao(usuario, &anterior, "DEPOSITO", valor,
                           "REAL", 0.0, 0.0)) {
        printf("Deposito nao foi concluido.\n");
        return;
    }

    printf("Deposito realizado. Novo saldo: R$ %.2f\n", usuario->reais);
}

static void sacar(Investidor *usuario) {
    Investidor anterior = *usuario;
    double valor;

    if (!lerDouble("Valor do saque em reais: R$ ", &valor) || valor <= 0.0) {
        printf("Valor invalido. Informe um numero maior que zero.\n");
        return;
    }

    valor = arredondarCentavos(valor);
    if (valor <= 0.0) {
        printf("O saque deve ser de pelo menos R$ 0,01.\n");
        return;
    }

    if (valor > usuario->reais) {
        printf("Saldo insuficiente. Seu saldo e R$ %.2f.\n", usuario->reais);
        return;
    }

    usuario->reais = arredondarCentavos(usuario->reais - valor);
    if (!confirmarOperacao(usuario, &anterior, "SAQUE", valor,
                           "REAL", 0.0, 0.0)) {
        printf("Saque nao foi concluido.\n");
        return;
    }

    printf("Saque realizado. Novo saldo: R$ %.2f\n", usuario->reais);
}

/* ---------- Compra e venda de criptomoedas ---------- */

static int escolherMoeda(void) {
    int moeda;

    printf("\nEscolha a criptomoeda:\n");
    printf("1 - Bitcoin (BTC)\n");
    printf("2 - Ethereum (ETH)\n");
    printf("3 - Ripple (XRP)\n");

    if (!lerInteiro("Opcao: ", &moeda) || moeda < 1 || moeda > 3) {
        printf("Opcao de criptomoeda invalida.\n");
        return 0;
    }
    return moeda;
}

static void comprar(Investidor *usuario, const Cotacoes *cotacoes) {
    Investidor anterior = *usuario;
    int moeda;
    double valor, cotacao, taxaPercentual, taxa, total, quantidade;
    double *saldoMoeda;
    const char *nomeMoeda;

    if (!autenticarSenha(usuario)) {
        return;
    }

    mostrarCotacoes(cotacoes);
    moeda = escolherMoeda();
    if (moeda == 0) {
        return;
    }

    if (!lerDouble("Valor que deseja investir em reais (sem a taxa): R$ ", &valor) ||
        valor <= 0.0) {
        printf("Valor invalido.\n");
        return;
    }

    valor = arredondarCentavos(valor);
    if (valor <= 0.0) {
        printf("O valor minimo e R$ 0,01.\n");
        return;
    }

    if (moeda == 1) {
        cotacao = cotacoes->bitcoin;
        taxaPercentual = 2.0;
        nomeMoeda = "BTC";
        saldoMoeda = &usuario->bitcoin;
    } else if (moeda == 2) {
        cotacao = cotacoes->ethereum;
        taxaPercentual = 1.0;
        nomeMoeda = "ETH";
        saldoMoeda = &usuario->ethereum;
    } else {
        cotacao = cotacoes->ripple;
        taxaPercentual = 1.0;
        nomeMoeda = "XRP";
        saldoMoeda = &usuario->ripple;
    }

    taxa = arredondarCentavos(valor * taxaPercentual / 100.0);
    total = arredondarCentavos(valor + taxa);

    if (total > usuario->reais) {
        printf("Saldo insuficiente. Compra + taxa = R$ %.2f; saldo = R$ %.2f.\n",
               total, usuario->reais);
        return;
    }

    quantidade = arredondarOitoCasas(valor / cotacao);
    if (quantidade <= 0.0 || !isfinite(quantidade) ||
        *saldoMoeda > MAX_VALOR - quantidade) {
        printf("Quantidade calculada fora do limite permitido.\n");
        return;
    }

    usuario->reais = arredondarCentavos(usuario->reais - total);
    *saldoMoeda = arredondarOitoCasas(*saldoMoeda + quantidade);

    if (!confirmarOperacao(usuario, &anterior, "COMPRA", quantidade,
                           nomeMoeda, cotacao, taxa)) {
        printf("Compra nao foi concluida.\n");
        return;
    }

    printf("\nCompra realizada: %.8f %s\n", quantidade, nomeMoeda);
    printf("Valor investido: R$ %.2f | Taxa: R$ %.2f | Total: R$ %.2f\n",
           valor, taxa, total);
    printf("Saldo em reais: R$ %.2f\n", usuario->reais);
}

static void vender(Investidor *usuario, const Cotacoes *cotacoes) {
    Investidor anterior = *usuario;
    int moeda;
    double quantidade, cotacao, taxaPercentual, bruto, taxa, liquido;
    double *saldoMoeda;
    const char *nomeMoeda;

    if (!autenticarSenha(usuario)) {
        return;
    }

    mostrarCotacoes(cotacoes);
    moeda = escolherMoeda();
    if (moeda == 0) {
        return;
    }

    if (moeda == 1) {
        cotacao = cotacoes->bitcoin;
        taxaPercentual = 3.0;
        nomeMoeda = "BTC";
        saldoMoeda = &usuario->bitcoin;
    } else if (moeda == 2) {
        cotacao = cotacoes->ethereum;
        taxaPercentual = 2.0;
        nomeMoeda = "ETH";
        saldoMoeda = &usuario->ethereum;
    } else {
        cotacao = cotacoes->ripple;
        taxaPercentual = 1.0;
        nomeMoeda = "XRP";
        saldoMoeda = &usuario->ripple;
    }

    if (!lerDouble("Quantidade que deseja vender: ", &quantidade) ||
        quantidade <= 0.0) {
        printf("Quantidade invalida.\n");
        return;
    }

    quantidade = arredondarOitoCasas(quantidade);
    if (quantidade <= 0.0) {
        printf("A quantidade minima e 0,00000001.\n");
        return;
    }

    if (quantidade > *saldoMoeda + 1e-12) {
        printf("Saldo insuficiente. Voce possui %.8f %s.\n",
               *saldoMoeda, nomeMoeda);
        return;
    }

    bruto = arredondarCentavos(quantidade * cotacao);
    taxa = arredondarCentavos(bruto * taxaPercentual / 100.0);
    liquido = arredondarCentavos(bruto - taxa);

    if (usuario->reais > MAX_VALOR - liquido) {
        printf("Valor da venda fora do limite permitido.\n");
        return;
    }

    *saldoMoeda = arredondarOitoCasas(*saldoMoeda - quantidade);
    if (*saldoMoeda < 0.000000005) {
        *saldoMoeda = 0.0;
    }
    usuario->reais = arredondarCentavos(usuario->reais + liquido);

    if (!confirmarOperacao(usuario, &anterior, "VENDA", quantidade,
                           nomeMoeda, cotacao, taxa)) {
        printf("Venda nao foi concluida.\n");
        return;
    }

    printf("\nVenda realizada: %.8f %s\n", quantidade, nomeMoeda);
    printf("Valor bruto: R$ %.2f | Taxa: R$ %.2f | Recebido: R$ %.2f\n",
           bruto, taxa, liquido);
    printf("Saldo em reais: R$ %.2f\n", usuario->reais);
}

/* ---------- Atualizacao das cotacoes ---------- */

static double variacaoAleatoria(void) {
    /* Percentual inteiro entre -5% e +5%, inclusive. */
    int percentual = (rand() % 11) - 5;
    return (double)percentual / 100.0;
}

static void atualizarCotacoes(Cotacoes *cotacoes) {
    Cotacoes anteriores = *cotacoes;

    cotacoes->bitcoin *= (1.0 + variacaoAleatoria());
    cotacoes->ethereum *= (1.0 + variacaoAleatoria());
    cotacoes->ripple *= (1.0 + variacaoAleatoria());

    if (!isfinite(cotacoes->bitcoin) || !isfinite(cotacoes->ethereum) ||
        !isfinite(cotacoes->ripple) || cotacoes->bitcoin <= 0.0 ||
        cotacoes->ethereum <= 0.0 || cotacoes->ripple <= 0.0 ||
        !salvarCotacoes(cotacoes)) {
        *cotacoes = anteriores;
        printf("Nao foi possivel atualizar/salvar as cotacoes.\n");
        return;
    }

    printf("Cotacoes atualizadas. Cada moeda variou entre -5%% e +5%%.\n");
    mostrarCotacoes(cotacoes);
}

/* ---------- Programa principal ---------- */

static void aguardarEnter(void) {
    printf("\nPressione ENTER para fechar esta janela...");
    fflush(stdout);
    /* Consome eventual resto de linha e aguarda uma entrada do usuario. */
    while (getchar() != '\n' && !feof(stdin)) {}
}

int main(void) {
    Investidor usuario;
    Cotacoes cotacoes;
    int opcao = 0;

    srand((unsigned int)time(NULL));

    printf("=============================================\n");
    printf("       EXCHANGE DE CRIPTOMOEDAS - CCP230\n");
    printf("=============================================\n");
    printf("Inicializando o sistema...\n\n");
    fflush(stdout);

    if (!criarArquivosIniciais()) {
        printf("\nERRO NA INICIALIZACAO: nao foi possivel preparar os arquivos.\n");
        printf("Verifique se a pasta do programa permite gravacao.\n");
        aguardarEnter();
        return EXIT_FAILURE;
    }

    if (!carregarInvestidor(&usuario)) {
        printf("\nERRO: dados invalidos no arquivo %s.\n", ARQ_USUARIO);
        printf("O arquivo deve conter nome, CPF (11 digitos), senha (6 digitos),\n");
        printf("saldo em reais e saldos BTC, ETH e XRP, nessa ordem.\n");
        aguardarEnter();
        return EXIT_FAILURE;
    }

    if (!carregarCotacoes(&cotacoes)) {
        printf("\nERRO: cotacoes invalidas no arquivo %s.\n", ARQ_COTACOES);
        printf("Confira se existem tres valores positivos no arquivo.\n");
        aguardarEnter();
        return EXIT_FAILURE;
    }

    printf("Arquivos carregados com sucesso.\n");
    printf("Cotacoes ficticias para simulacao academica.\n");
    printf("Investidor inicial: Investidor Teste\n");
    printf("CPF de teste: 12345678900 | Senha: 123456\n");
    fflush(stdout);

    if (!login(&usuario)) {
        printf("\nLogin nao realizado. O programa sera encerrado.\n");
        aguardarEnter();
        return EXIT_FAILURE;
    }

    do {
        printf("\n=============== MENU PRINCIPAL ===============\n");
        printf("1. Consultar saldo\n");
        printf("2. Consultar extrato\n");
        printf("3. Depositar reais\n");
        printf("4. Sacar reais\n");
        printf("5. Comprar criptomoedas\n");
        printf("6. Vender criptomoedas\n");
        printf("7. Atualizar cotacoes\n");
        printf("8. Sair\n");

        if (!lerInteiro("Escolha uma opcao: ", &opcao)) {
            if (feof(stdin)) {
                printf("\nEntrada encerrada. Saindo com seguranca.\n");
                break;
            }
            printf("Entrada invalida. Digite um numero de 1 a 8.\n");
            continue;
        }

        switch (opcao) {
            case 1: consultarSaldo(&usuario); break;
            case 2: consultarExtrato(&usuario); break;
            case 3: depositar(&usuario); break;
            case 4: sacar(&usuario); break;
            case 5: comprar(&usuario, &cotacoes); break;
            case 6: vender(&usuario, &cotacoes); break;
            case 7: atualizarCotacoes(&cotacoes); break;
            case 8: printf("Dados gravados nos arquivos .txt. Ate logo!\n"); break;
            default: printf("Opcao invalida. Escolha de 1 a 8.\n");
        }
    } while (opcao != 8);

    aguardarEnter();
    return EXIT_SUCCESS;
}
