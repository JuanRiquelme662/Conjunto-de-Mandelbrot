




//a linha abaixo e o necessario para virar opemmp, mas vou ver isso melhor amanha
//#pragma omp parallel for num_threads(num_threads)
void mandlebrot_serial(){
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

        }
    }

}

int main(){
    return 0;
}