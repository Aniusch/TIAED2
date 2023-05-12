#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define NUM_VIAS 3
#define NUM_PISTAS 2 // pistas por via
#define MAX_VEICULOS 20 // quantidade maxima de veiculos em uma via por vez
#define PRIORIDADE_PROB 1 // chance de ser veiculo prioritario %
#define REFLEXO_CARRO 0.5 // tempo de reacao de um motorista de carro
#define ONIBUS_PROB 20 // chance de um veiculo ser onibus %
#define CARRO_PROB 80 // chance de ser carro %
#define REFLEXO_ONIBUS (2 * REFLEXO_CARRO) // onibus tem o reflexo 2 vzs mais lento
#define COLISAO_PROB 5 // chance de colisao 1 - 100 %
#define MULTIPLICADOR_COLISAO 1.25 // multiplicador de tempo caso haja colisao
#define MULTIPLICADOR_PRIORITARIO 1.55 // multiplicador de tempo de espera caso haja prioritario
#define TIPO_VEICULOS 3// quantos tipos diferentes de veiculos ha?
#define VAZIO -1 // criar o array com maior facilidade
#define TAMANHO_ROT 6 //tamanho da rotatoria

typedef enum{CARRO, ONIBUS, PRIORITARIO}tipoVeiculo;
    
typedef struct Veiculo
{   
    tipoVeiculo tipo; // eh carro/onibus/prioritario?
    double reflexo; // reflexo do motorista
    double colisao; //chance de colisao
} Veiculo;

typedef struct Via
{ 
    int via[NUM_PISTAS][MAX_VEICULOS]; // pistas com maximo de 50 veiculos por faixa
    int numVeiculosPista[NUM_PISTAS]; //numero de veiculos por pista
    int queue; // quantos veiculos na fila para entrar na pista?
} Via;

typedef struct Rot
{   
    int maxVeiculos; //maximo veiculos dentro da rotatoria
    int dentroRot[TAMANHO_ROT]; // vetor que representa a rotatoria
    int veiculosDentro; //veiculos dentro da rotatoria atualmente
    double tempoDentroRotatoria; // tempo total de veiculos para passar dentro rotatoria
    int veiculosPassados; //quantos veiculos ja passaram pela rotatoria
    
} Rot; 

typedef struct Controle
{
    int id; // id simulacao
    int semaforo;
    int pistaPrioritaria;
    double tempoTotal; // tempo de espera medio
    int prioritarios; //quantos veiculos prioritarios no total?
    int onibus; // quantos onibus passaram no total?
    int carros; //quantos carros passaram no total?
    int veiculosPassados;
    int veiculosTransito; // veiculos em transiot no momento
    double multiplicadorT; //multiplicador de tempo universal
    int queueTotal; // todos os veiculos em queue
    int acidentes; //quantos acidentes ocorreram
    int veiculosViaMax;

} Controle;


//funcoes prototipo:

int geradorRandom(int max, int min);
void rotatoriaSort(int *arr);
void viaSort(Via *vias, int via, int pista);
void escolherSimulacao(Controle *controle, char *argv[]);
void iniciarControle(Controle *controle);
void iniciarVeiculos(Veiculo *veiculos, Controle *controle);
void iniciarVias(Via *vias, Controle *controle);
void iniciarRotatoria(Rot *rotatoria);
void simulacao(Veiculo *veiculo, Via *vias, Rot *rotatoria, Controle *controle);
void entrarRotatoria(Via *vias, Controle *controle, Rot *rotatoria, Veiculo *veiculos);
void movimentarRotatoria(Controle *controle, Rot *rotatoria, Veiculo *veiculos);
void movimentarVia(Via *vias, int via, int pista, Controle *controle);
void removerRotatoria(Controle *controle, Rot *rotatoria);
void adcionarVeiculo(Via *vias, int via, int pista, Controle *controle);
void chegadaVeiculo(Veiculo *veiculos, Via *vias, Controle *controle);
void acidente(int v, Controle *controle, Rot *rotatoria, Veiculo *veiculos);
void processamentoDeDados(Rot *rotatoria, Controle *controle);

//driver code
int main(int argc, char *argv[]){
    
    if (argc < 3) {
    return 1;
    }
    
    srand(time(NULL));

    Veiculo *veiculos;
    veiculos = malloc(sizeof(Veiculo) * TIPO_VEICULOS);
    Via *vias;
    vias = malloc(sizeof(Via) * NUM_VIAS);
    Rot *rotatoria;
    rotatoria = malloc(sizeof(Rot));
    Controle *controle;
    controle = malloc(sizeof(Controle));

    escolherSimulacao(controle, argv); 

    //roda todas as simulacoes de uma vez
    if(controle->id != 5){
        iniciarControle(controle);
        iniciarVeiculos(veiculos, controle);
        iniciarVias(vias, controle);
        iniciarRotatoria(rotatoria);
        simulacao(veiculos, vias, rotatoria, controle);
    }
    else{
        int i = 1;
        while(i < 5){
            controle->id = i;
            iniciarControle(controle);
            iniciarVeiculos(veiculos, controle);
            iniciarVias(vias, controle);
            iniciarRotatoria(rotatoria);
            i++;
            simulacao(veiculos, vias, rotatoria, controle);
        }
    }
    free(veiculos);
    free(vias);
    free(rotatoria);
    free(controle);
    return 0;
}

//funcao que gera um numero aleatorio
int geradorRandom(int max, int min){
    return ((rand() % (max + 1)) + min);
}

//sort para movimentar a rotatoria
void rotatoriaSort(int *arr){
    int j = TAMANHO_ROT - 1;
    for (int i = 0; i < TAMANHO_ROT - 1; i++) {
        if (arr[j] <= arr[j - 1]) {
          int temp = arr[j];
            arr[j] = arr[j - 1];
            arr[j - 1] = temp;
            j--;
        }
    }
    /* visualizar o sort
    printf("rotatoria : [ ");
    for(int i = 0; i < TAMANHO_ROT - 1; i++){
        printf("%d ", arr[i]);
    }
    printf("]\n");
    */
}

//sort para movimentar as vias
void viaSort(Via *vias, int via, int pista){
    int tamanhoArray = MAX_VEICULOS;

    for (int i = 0; i < tamanhoArray - 1; i++) {
        if (vias[via].via[pista][i] <= vias[via].via[pista][i + 1]){
            int temp = vias[via].via[pista][i];
            vias[via].via[pista][i] = vias[via].via[pista][i + 1];
            vias[via].via[pista][i + 1] = temp;
        }
    }
    /* visualizar o sort
    printf("via : [ ");
    for(int i = 0; i < tamanhoArray - 1; i++){
        printf("%d ", vias[via].via[pista][i]);
    }
    printf("]\n");
    */
}

//recebe input do usuario para iniciar a simulacao
void escolherSimulacao(Controle *controle, char *argv[]){
    int escolha = atoi(argv[1]);
    controle->veiculosViaMax = atoi(argv[2]);
    while(true){
        if(0 < escolha && escolha <= 5){
            controle->id = escolha;
            break;
        }
        else{
            exit(1);
        }
    }
}

//inicia o controle de semaforos, pistas prioritarias, tempo total, etc
void iniciarControle(Controle *controle){
    controle->tempoTotal = 0;
    controle->veiculosPassados= 0;
    controle->carros = 0;
    controle->onibus = 0;
    controle->prioritarios = 0;
    controle->veiculosTransito = 0;
    controle->multiplicadorT = 1.0;
    controle->queueTotal = 0;
    controle->acidentes = 0;

    while(true){
        if(controle->id == 1){
            controle->pistaPrioritaria = false;
            controle->semaforo = false;
            break;
        }
        else if (controle->id == 2){   
            controle->pistaPrioritaria = true;    
            controle->semaforo = false;
            break;
        }
        else if (controle->id == 3){   
            controle->pistaPrioritaria = false;    
            controle->semaforo = true;
            break;
        }
        else if (controle->id == 4){   
            controle->pistaPrioritaria = true;    
            controle->semaforo = true;
            break;
        }
        else{
            exit(1);
        }
    }
}

//ínicia os veiculos
void iniciarVeiculos(Veiculo *veiculos, Controle *controle){
    double colisao;
    if(controle->semaforo == true){
        colisao = 0.0;
    }
    else{
        colisao = COLISAO_PROB;
    }
        
    veiculos[0].tipo =  CARRO;
    veiculos[0].colisao = colisao;
    veiculos[0].reflexo = REFLEXO_CARRO;

    veiculos[1].tipo = ONIBUS;
    veiculos[1].colisao = colisao;
    veiculos[1].reflexo = REFLEXO_ONIBUS;

    veiculos[2].tipo = PRIORITARIO;
    veiculos[2].reflexo = REFLEXO_CARRO;
    veiculos[2].colisao = 0.0;
}

//inicializar vias
void iniciarVias(Via *vias, Controle *controle){
    int i, k, j;

    //inicializa as vias vazias
    for(k = 0; k < NUM_VIAS; k++){
        for(i = 0; i < NUM_PISTAS; i++){
            for(j = 0; j < MAX_VEICULOS; j++){
                vias[k].via[i][j] = VAZIO; 
            }                
        }
    }

    //inicia o contador de veiculos em espera e em transito
    for(i = 0; i < NUM_VIAS; i++){
        vias[i].queue = 0;
        vias[i].numVeiculosPista[0] = MAX_VEICULOS;
        vias[i].numVeiculosPista[1] =  MAX_VEICULOS;
        controle->veiculosTransito += 2 * MAX_VEICULOS;
    }

    //adciona os veiculos inciais
    int veiculo;
    for(k = 0; k < NUM_VIAS; k++){
        for(i = 0; i < NUM_PISTAS; i++){
            for(j = 0; j < vias[k].numVeiculosPista[i]; j++){
                veiculo = geradorRandom(100, 2); // iniciar sem prioritario
                if(veiculo > 80){
                    vias[k].via[i][j] = ONIBUS;
                }
                else{
                    vias[k].via[i][j] = CARRO;
                }
            }                
        }
    }
    /*mostra veiculos inciais
    printf("Veiculos Iniciais:\n");
    for(k = 0; k < NUM_VIAS; k++){
        printf("VIA[%d]:\n", k);
        for(i = 0; i < NUM_PISTAS; i++){
            printf(" PISTA[%d] : [ ", i);
            for(j = 0; j < MAX_VEICULOS; j++){
                printf("%d ", vias[k].via[i][j]);
            }
            printf("]\n");                
        }
    }
    printf("veiculos em transito inicialmente: %d\n", controle->veiculosTransito);
    */
}

//inicia a rotatoria
void iniciarRotatoria(Rot *rotatoria){
    rotatoria->maxVeiculos = TAMANHO_ROT;
    rotatoria->veiculosDentro = 0;
    rotatoria->veiculosPassados = 0;
    rotatoria->tempoDentroRotatoria = 0.0;
    for(int i = 0; i < rotatoria->maxVeiculos; i++){
        rotatoria->dentroRot[i] = -1;
    }
    /* mostra o inicio
    printf("Rotatoria inicial : [ ");
    for(int i = 0; i < rotatoria->maxVeiculos; i++){
        printf("%d ", rotatoria->dentroRot[i]);
    }
    printf("]\n");
    */
}

//simula a entrada na rotatoria
void entrarRotatoria(Via *vias, Controle *controle, Rot *rotatoria, Veiculo *veiculos){
    if(rotatoria->veiculosDentro < rotatoria->maxVeiculos){
        int via = geradorRandom(2, 0);
        int pista = geradorRandom(1, 0);    
        do{
           if(rotatoria->dentroRot[0] == -1){ // se a primeira posicao tiver livre, add carro
                if (vias[via].via[pista][0] == -1){// caso nao tenha carro na primeira posicao, organize a via
                    movimentarVia(vias, via, pista, controle);
                }
                if(vias[via].via[pista][0] != -1){ // se tiver carro para add
                    rotatoria->dentroRot[0] = vias[via].via[pista][0];
                    acidente(rotatoria->dentroRot[0], controle, rotatoria, veiculos); // checa por acidentes
                    vias[via].via[pista][0] = -1;
                    vias[via].numVeiculosPista[pista]--;
                    if(vias[via].numVeiculosPista[pista] == 0){
                        adcionarVeiculo(vias, via, pista, controle); // adciona mais carros ao vetor, caso possivel, tirando os da fila
                    }
                    movimentarVia(vias, via, pista, controle); // sort 
                    int randomTempoEntrada = (veiculos[rotatoria->dentroRot[0]].reflexo * geradorRandom(2, 1)); // tempo dentro
                    rotatoria->tempoDentroRotatoria += randomTempoEntrada;

                    if(rotatoria->dentroRot[0] != -1){ // confirma que entrou veiculo e conta
                        rotatoria->veiculosDentro++;
                    }
                }
            }
            else{
                controle->tempoTotal += (1 * controle->multiplicadorT); // controla o tempo medio de espera
            }
            movimentarRotatoria(controle, rotatoria, veiculos);
        }while ((controle->semaforo == true) && (controle->veiculosPassados % 15 != 0));   
    }
}

//movimentar rotatoria
void movimentarRotatoria(Controle *controle, Rot *rotatoria, Veiculo *veiculos){
    if(rotatoria->dentroRot[rotatoria->maxVeiculos - 1] == -1){
        rotatoriaSort(rotatoria->dentroRot);
        rotatoria->tempoDentroRotatoria++;
    }
    if(rotatoria->dentroRot[rotatoria->maxVeiculos - 1] != -1){
        removerRotatoria(controle, rotatoria);
    }
}

//movimentar a via
void movimentarVia(Via *vias, int via, int pista, Controle *controle){
    if(vias[via].via[pista][0] == -1){
        viaSort(vias, via, pista);
        controle->tempoTotal = controle->tempoTotal + (controle->multiplicadorT);
    }
}

//remover veiculo da rotatoria
void removerRotatoria(Controle *controle, Rot *rotatoria){
    if(rotatoria->dentroRot[rotatoria->maxVeiculos - 1] == PRIORITARIO){
        controle->prioritarios++;

        if(controle->pistaPrioritaria == false){
            controle->multiplicadorT = controle->multiplicadorT / MULTIPLICADOR_PRIORITARIO;
        }
    }
    else if (rotatoria->dentroRot[rotatoria->maxVeiculos - 1] == CARRO)
    {
        controle->carros++;
    }
    else if(rotatoria->dentroRot[rotatoria->maxVeiculos - 1] == ONIBUS){
        controle->onibus++;
    }
    rotatoria->dentroRot[rotatoria->maxVeiculos - 1] = -1;
    controle->veiculosPassados++;
    controle->veiculosTransito--;
    rotatoria->veiculosDentro--;
}

//simula a chegada de mais carros (por ser horario de pico, tem uma chance garatida de chegar carro)
void chegadaVeiculo(Veiculo *veiculos, Via *vias, Controle *controle){
    int novosVeiculos;
    int queue;

    //gera quantos veiculos vao chegar e os adiciona na fila
    for(int i = 0; i < NUM_VIAS; i++){
        queue = geradorRandom(20 , 10); //vai chegar de 10 a 20 veiculos por vez
        vias[i].queue += queue;
        controle->queueTotal += queue;
    }
}

//tira carros da fila e adciona no vetor via
void adcionarVeiculo(Via *vias, int via, int pista, Controle *controle){
    int novoVeiculo;
    int prob;
    for(int i = 0; i < MAX_VEICULOS; i++){
        prob = geradorRandom(100, 1);
        if(prob < 80){
            novoVeiculo = CARRO;
        }
        else if(prob > 80){
            novoVeiculo = ONIBUS;
        }
        else if(prob == 80){
            novoVeiculo = PRIORITARIO;
        }

        vias[via].via[pista][i] = novoVeiculo;
        vias[via].queue--;
        controle->queueTotal--;
        vias[via].numVeiculosPista[pista]++;
        controle->veiculosTransito++;
        if(novoVeiculo == PRIORITARIO && controle->pistaPrioritaria == false){
          controle->multiplicadorT = controle->multiplicadorT * MULTIPLICADOR_PRIORITARIO;
        }
    }
}

//simula acidente
void acidente(int v, Controle *controle, Rot *rotatoria, Veiculo *veiculos){
    if(controle->semaforo == false){
        int acidenteChance = geradorRandom(100, 1);
        if(acidenteChance <= veiculos[v].colisao){
            controle->multiplicadorT = controle->multiplicadorT * MULTIPLICADOR_COLISAO;
            controle->acidentes++;
        }
    }
}

//realiza o processamento de dados
void processamentoDeDados(Rot *rotatoria, Controle *controle){
    double tempoMedioNot;
    double tempoRotatoria;
    double tempoTotalNot;

    tempoMedioNot = (controle->tempoTotal / controle->veiculosPassados) / 100;
    tempoTotalNot = controle->tempoTotal / 100;
    tempoRotatoria = (rotatoria->tempoDentroRotatoria) / controle->veiculosPassados;

    printf("DADOS DA SIMULACAO (%d):\n", controle->id);
    printf("-------------------\n");
    printf("Veiculos que passaram:\n");
    printf("Carros: %d\tOnibus: %d\tPrioritarios: %d\n", controle->carros, controle->onibus, controle->prioritarios);
    printf("Tempo de espera medio nas vias: %.5lf * 10^3t\n", tempoMedioNot);
    printf("Tempo de espera total nas vias: %.5lft * 10^3\n", tempoTotalNot);
    printf("Tempo total para todos os veiculos passarem pela rotatoria: %.2lft\n", rotatoria->tempoDentroRotatoria);
    printf("Tempo medio para passar pela rotatoria: %.5lft\n", tempoRotatoria);
    printf("Acidentes: %d\n", controle->acidentes);
}

//rodar simulacao
void simulacao(Veiculo *veiculos, Via *vias, Rot *rotatoria, Controle *controle){
    int veiculosMaximos = NUM_VIAS * controle->veiculosViaMax;
    while(controle->veiculosPassados != veiculosMaximos){
        entrarRotatoria(vias, controle, rotatoria, veiculos);
        chegadaVeiculo(veiculos, vias, controle);
    }
    processamentoDeDados(rotatoria, controle);
}