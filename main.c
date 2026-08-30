#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <pthread.h>
#include <time.h>

//vou usar clock_gettime, porque so o clock mede o tempo da cpu
typedef struct {
    unsigned char *pixels;
    int largura;
    int altura;
    int max_num_interacoes;
    int linha_inicio;   
    int linha_fim;      

} ThreadArgs;

//nova struct para tratar os thrads que vai funcionar dinamicamente
//nesse caso os trheads terao um contador compartilhado para que saibam o proximo passo da execucao
typedef struct {
    unsigned char *pixels;
    int largura;
    int altura;
    int max_num_interacoes;
    int *proxima_linha;        
    //mutex serve para nao deixar duas threads mexerem no mesmo lugar ao mesmo tempo
    pthread_mutex_t *mutex;    
} ThreadArgs_Dinamico;
//a linha abaixo e o necessario para virar opemmp, mas vou ver isso melhor amanha
//#pragma omp parallel for num_threads(num_threads)
void mandelbrot_serial(unsigned char *pixels, int largura, int altura, int max_num_interacoes){
    for(int linha = 0; linha < altura; linha++){
        for(int coluna = 0; coluna < largura; coluna++){

            //calculo do c desse pixel
            double cr = -2.0 + (coluna / (double)(largura - 1)) * 3.0;
            double ci = -1.5 + (linha / (double)(altura - 1)) * 3.0;

            //zera as variaves
            double zr = 0, zi = 0;
            int num_interacoes = 0;
            //esse (zr*zr + zi*zi) <= 4 e uma forma simplificada de ver se o numero saiu do conjunto de mandlebrot
            //sem ter que depender de raiz quadrada
            while (num_interacoes < max_num_interacoes && (zr*zr + zi*zi) <= 4) {

                double zr_novo = zr*zr - zi*zi + cr;
                double zi_novo = 2*zr*zi + ci;
                zr = zr_novo;
                zi = zi_novo;
                num_interacoes++;

            }
            unsigned char intensidade = (unsigned char)((num_interacoes / (double)max_num_interacoes) * 255);
            pixels[linha * largura + coluna] = intensidade;
        }
    }

}

void mandelbrot_openmp(unsigned char *pixels, int largura, int altura, int max_num_interacoes, int num_threads){
    #pragma omp parallel for num_threads(num_threads)
    for(int linha = 0; linha < altura; linha++){
        for(int coluna = 0; coluna < largura; coluna++){

            //calculo do c desse pixel
            double cr = -2.0 + (coluna / (double)(largura - 1)) * 3.0;
            double ci = -1.5 + (linha / (double)(altura - 1)) * 3.0;

            //zera as variaves
            double zr = 0, zi = 0;
            int num_interacoes = 0;
            //esse (zr*zr + zi*zi) <= 4 e uma forma simplificada de ver se o numero saiu do conjunto de mandlebrot
            //sem ter que depender de raiz quadrada
            while (num_interacoes < max_num_interacoes && (zr*zr + zi*zi) <= 4) {

                double zr_novo = zr*zr - zi*zi + cr;
                double zi_novo = 2*zr*zi + ci;
                zr = zr_novo;
                zi = zi_novo;
                num_interacoes++;

            }
            unsigned char intensidade = (unsigned char)((num_interacoes / (double)max_num_interacoes) * 255);
            pixels[linha * largura + coluna] = intensidade;
        }
    }
    
}

//funcao responsavel por salvar o resultado dos calculos em um arquivo real
int save_pgm(const char *filename, unsigned char *pixels, int largura, int altura) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "Erro: falha ao criar o arquivo %s\n", filename);
        return 0; // falhou
    }

    for (int linha = 0; linha < altura; linha++) {
        for (int coluna = 0; coluna < largura; coluna++) {
            fprintf(f, "%d ", pixels[linha * largura + coluna]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 1; // sucesso
}

//void* porque e um ponteiro generico (de qualquer tipo)
void *calcula_bloco(void *arg) {
    ThreadArgs *args = (ThreadArgs *) arg;

    for (int linha = args->linha_inicio; linha < args->linha_fim; linha++) {
        for (int coluna = 0; coluna < args->largura; coluna++) {

            double cr = -2.0 + (coluna / (double)(args->largura - 1)) * 3.0;
            double ci = -1.5 + (linha / (double)(args->altura - 1)) * 3.0;

            double zr = 0, zi = 0;
            int num_interacoes = 0;

            while (num_interacoes < args->max_num_interacoes && (zr*zr + zi*zi) <= 4) {
                double zr_novo = zr*zr - zi*zi + cr;
                double zi_novo = 2*zr*zi + ci;
                zr = zr_novo;
                zi = zi_novo;
                num_interacoes++;
            }

            unsigned char intensidade = (unsigned char)((num_interacoes / (double)args->max_num_interacoes) * 255);
            args->pixels[linha * args->largura + coluna] = intensidade;

        }
    }

    return NULL;
}

void mandelbrot_pthreads1(unsigned char *pixels, int largura, int altura, int max_num_interacoes, int num_threads) {
    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    int linhas_por_thread = altura / num_threads;

    for (int t = 0; t < num_threads; t++) {
        args[t].pixels = pixels;
        args[t].largura = largura;
        args[t].altura = altura;
        args[t].max_num_interacoes = max_num_interacoes;
        args[t].linha_inicio = t * linhas_por_thread;
        args[t].linha_fim = (t == num_threads - 1) ? altura : (t + 1) * linhas_por_thread;

        pthread_create(&threads[t], NULL, calcula_bloco, &args[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

void *calcula_dinamico(void *arg) {
    ThreadArgs_Dinamico *args = (ThreadArgs_Dinamico *) arg;

    while (1) {
        pthread_mutex_lock(args->mutex);
        int linha = *(args->proxima_linha);
        (*(args->proxima_linha))++;
        pthread_mutex_unlock(args->mutex);

        if (linha >= args->altura) {
            break;
        }

        // AQUI: só o for de COLUNA, usando a "linha" que já veio do contador
        for (int coluna = 0; coluna < args->largura; coluna++) {

            double cr = -2.0 + (coluna / (double)(args->largura - 1)) * 3.0;
            double ci = -1.5 + (linha / (double)(args->altura - 1)) * 3.0;

            double zr = 0, zi = 0;
            int num_interacoes = 0;

            while (num_interacoes < args->max_num_interacoes && (zr*zr + zi*zi) <= 4) {
                double zr_novo = zr*zr - zi*zi + cr;
                double zi_novo = 2*zr*zi + ci;
                zr = zr_novo;
                zi = zi_novo;
                num_interacoes++;
            }

            unsigned char intensidade = (unsigned char)((num_interacoes / (double)args->max_num_interacoes) * 255);
            args->pixels[linha * args->largura + coluna] = intensidade;
        }
    }

    return NULL;
}

void mandelbrot_pthreads2(unsigned char *pixels, int largura, int altura, int max_num_interacoes, int num_threads) {
    pthread_t threads[num_threads];
    ThreadArgs_Dinamico args[num_threads];

    //encrementa o contador compartilhado
    int proxima_linha = 0;               
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);    

    for (int t = 0; t < num_threads; t++) {
        args[t].pixels = pixels;
        args[t].largura = largura;
        args[t].altura = altura;
        args[t].max_num_interacoes = max_num_interacoes;
        args[t].proxima_linha = &proxima_linha;   
        args[t].mutex = &mutex;                   

        pthread_create(&threads[t], NULL, calcula_dinamico, &args[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    //da como se fosse um free
    pthread_mutex_destroy(&mutex);       
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s largura altura max_iteracoes num_threads\n", argv[0]);
        return 1;
    }

    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_num_interacoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    if (largura <= 0 || altura <= 0 || max_num_interacoes <= 0 || num_threads <= 0) {
        fprintf(stderr, "Erro: parâmetros devem ser inteiros positivos\n");
        return 1;
    }

    //unsigned char nao tem sinal, sou seja vai de 0 a 255,
    unsigned char *pixels = malloc(largura * altura * sizeof(unsigned char));
    if (pixels == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória\n");
        return 1;
    }

    FILE *times_file = fopen("times.txt", "w");
    if (times_file == NULL) {
        fprintf(stderr, "Erro: falha ao criar times.txt\n");
        free(pixels);
        return 1;
    }

    struct timespec inicio, fim;
    double tempo;

    // ---Serial---
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_serial(pixels, largura, altura, max_num_interacoes);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    fprintf(times_file, "Serial: %f segundos\n", tempo);

    if (!save_pgm("mandelbrot_jrxs_serial.pgm", pixels, largura, altura)) {
        free(pixels);
        fclose(times_file);
        return 1;
    }

    // ---OpenMP---
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_openmp(pixels, largura, altura, max_num_interacoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    fprintf(times_file, "OpenMP: %f segundos\n", tempo);

    if (!save_pgm("mandelbrot_jrxs_openmp.pgm", pixels, largura, altura)) {
        free(pixels);
        fclose(times_file);
        return 1;
    }

    // ---Pthreads1---
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_pthreads1(pixels, largura, altura, max_num_interacoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    fprintf(times_file, "Pthreads1: %f segundos\n", tempo);

    if (!save_pgm("mandelbrot_jrxs_pthreads1.pgm", pixels, largura, altura)) {
        free(pixels);
        fclose(times_file);
        return 1;
    }

    // ---Pthreads2---
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_pthreads2(pixels, largura, altura, max_num_interacoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    fprintf(times_file, "Pthreads2: %f segundos\n", tempo);

    if (!save_pgm("mandelbrot_jrxs_pthreads2.pgm", pixels, largura, altura)) {
        free(pixels);
        fclose(times_file);
        return 1;
    }

    fclose(times_file);
    free(pixels);
    return 0;
}