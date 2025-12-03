#pragma once
#include <vector>
#include "dataSample.h"

class Buffer
{
private:
    size_t capacity;                      // Tamanho fixo (N)
    std::vector<dataSample> dados;        // Armazena os elementos
    size_t head;                          // Índice de escrita (circular)
    size_t count;                         // Quantidade atual

public:
    Buffer(size_t N)
        : capacity(N), dados(N), head(0), count(0) {}

    // Verifica se o buffer está cheio
    bool isFull() const {
        return count == capacity;
    }

    // Método push: sobrescreve o elemento mais antigo (buffer circular)
    void push(const dataSample& sample)
    {
        dados[head] = sample;        // Escreve no índice atual
        head = (head + 1) % capacity; // Avança circularmente

        if (count < capacity)
            count++;
    }

    // Método addData: NÃO aceita sobrescrever; só adiciona se houver espaço
    bool addData(const dataSample& sample)
    {
        if (isFull())
            return false;

        dados[head] = sample;
        head = (head + 1) % capacity;
        count++;

        return true;
    }

    // Retorna TODAS as amostras válidas na ordem correta (FIFO)
    std::vector<dataSample> getAmostras() const
    {
        std::vector<dataSample> out;
        out.reserve(count);

        size_t idx = (head + capacity - count) % capacity;

        for (size_t i = 0; i < count; i++)
            out.push_back(dados[(idx + i) % capacity]);

        return out;
    }

    size_t size() const { return count; }
    size_t maxSize() const { return capacity; }
};
