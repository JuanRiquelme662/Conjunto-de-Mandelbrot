#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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

int main(int argc, char *argv[]) {
    //valida para ver se a quantidade de argumentos e valida
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

    //unsigned char nao tem sinal, sou seja vai de 0 a 255, justamente os valores RGB
    unsigned char *pixels = malloc(largura * altura * sizeof(unsigned char));
    if (pixels == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória\n");
        return 1;
    }

    // --- Serial ---
    mandelbrot_serial(pixels, largura, altura, max_num_interacoes);

    if (!save_pgm("mandelbrot_jrxs_serial.pgm", pixels, largura, altura)) {
        free(pixels);
        return 1;
    }

    // --- OpenMP ---
    mandelbrot_openmp(pixels, largura, altura, max_num_interacoes, num_threads);

    if (!save_pgm("mandelbrot_jrxs_openmp.pgm", pixels, largura, altura)) {
        free(pixels);
        return 1;
    }

    free(pixels);
    return 0;
}