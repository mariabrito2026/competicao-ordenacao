/*
 * Competição de Ordenação: O Desafio dos Milissegundos
 *
 * Estratégia: Counting Sort (ordenação por contagem), O(n + k), onde
 * n = quantidade de elementos e k = amplitude dos valores (max - min + 1).
 * Nos dados da competição k = 2.500.000 e n = 10.000.000, então é bem mais
 * rápido que algoritmos O(n log n) como Quick Sort ou Merge Sort.
 *
 * Se a amplitude for muito grande (k > LIMITE_K), usa-se Radix Sort LSD
 * como alternativa, para não estourar a memória.
 *
 * Compilação:  gcc -O3 -march=native -o ordenacao ordenacao.c
 * Execução:    ./ordenacao ListahAhSerOrdenada.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define LIMITE_K 100000000u  /* acima disso usa Radix Sort */

/* ---------- Leitura do arquivo (NÃO entra na medição de tempo) ---------- */
static uint32_t *ler_arquivo(const char *caminho, size_t *n_out) {
    FILE *f = fopen(caminho, "rb");
    if (!f) { perror("fopen"); exit(1); }
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc((size_t)tam + 1);
    if (!buf || fread(buf, 1, (size_t)tam, f) != (size_t)tam) {
        fprintf(stderr, "Erro ao ler o arquivo\n"); exit(1);
    }
    fclose(f);
    buf[tam] = '\0';

    size_t cap = (size_t)tam / 2 + 1;  /* no mínimo 2 bytes por número */
    uint32_t *v = malloc(cap * sizeof *v);
    if (!v) { fprintf(stderr, "Sem memória\n"); exit(1); }

    size_t n = 0;
    char *p = buf, *fim = buf + tam;
    while (p < fim) {
        while (p < fim && (*p < '0' || *p > '9')) p++;
        if (p >= fim) break;
        uint32_t x = 0;
        while (p < fim && *p >= '0' && *p <= '9') x = x * 10 + (uint32_t)(*p++ - '0');
        v[n++] = x;
    }
    free(buf);
    *n_out = n;
    return v;
}

/* ---------- Radix Sort LSD (alternativa para amplitudes enormes) ---------- */
static void radix_sort(uint32_t *v, size_t n) {
    uint32_t *tmp = malloc(n * sizeof *tmp);
    if (!tmp) { fprintf(stderr, "Sem memória\n"); exit(1); }
    uint32_t *src = v, *dst = tmp;
    for (int shift = 0; shift < 32; shift += 8) {
        size_t cont[256] = {0};
        for (size_t i = 0; i < n; i++) cont[(src[i] >> shift) & 0xFF]++;
        size_t soma = 0;
        for (int b = 0; b < 256; b++) { size_t c = cont[b]; cont[b] = soma; soma += c; }
        for (size_t i = 0; i < n; i++) dst[cont[(src[i] >> shift) & 0xFF]++] = src[i];
        uint32_t *t = src; src = dst; dst = t;
    }
    /* após 4 passadas (par), o resultado já está em v */
    free(tmp);
}

/* ---------- Função de ordenação (ESTA parte é cronometrada) ---------- */
static void ordenarVetor(uint32_t *v, size_t n) {
    if (n < 2) return;

    uint32_t mn = v[0], mx = v[0];
    for (size_t i = 1; i < n; i++) {
        if (v[i] < mn) mn = v[i];
        if (v[i] > mx) mx = v[i];
    }
    uint64_t k = (uint64_t)mx - mn + 1;

    if (k > LIMITE_K) { radix_sort(v, n); return; }

    uint32_t *cont = calloc((size_t)k, sizeof *cont);
    if (!cont) { radix_sort(v, n); return; }

    for (size_t i = 0; i < n; i++) cont[v[i] - mn]++;

    size_t pos = 0;
    for (uint64_t val = 0; val < k; val++) {
        uint32_t c = cont[val];
        uint32_t x = (uint32_t)(val + mn);
        for (uint32_t j = 0; j < c; j++) v[pos++] = x;
    }
    free(cont);
}

/* ---------- Verificação (fora da medição) ---------- */
static int verificar(const uint32_t *v, size_t n, uint64_t soma_original) {
    uint64_t soma = 0;
    for (size_t i = 0; i < n; i++) {
        soma += v[i];
        if (i > 0 && v[i - 1] > v[i]) return 0;  /* fora de ordem */
    }
    return soma == soma_original;  /* nenhum elemento perdido/alterado */
}

int main(int argc, char **argv) {
    const char *caminho = argc > 1 ? argv[1] : "ListahAhSerOrdenada.txt";

    size_t n;
    uint32_t *vetor = ler_arquivo(caminho, &n);
    printf("Elementos lidos: %zu\n", n);

    uint64_t soma_original = 0;
    for (size_t i = 0; i < n; i++) soma_original += vetor[i];

    /* --- cronômetro: SOMENTE a ordenação --- */
    clock_t inicio = clock();
    ordenarVetor(vetor, n);
    clock_t fim = clock();
    double tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
    /* ------------------------------------- */

    printf("Tempo de ordenação: %f segundos (%f ms)\n", tempo_gasto, tempo_gasto * 1000.0);
    printf("Verificação: %s\n", verificar(vetor, n, soma_original)
           ? "OK (ordem crescente, sem perda de elementos)" : "FALHOU");

    free(vetor);
    return 0;
}
