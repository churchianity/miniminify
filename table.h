
#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <stdint.h>


template <class K, class V>
struct TableEntry {
    TableEntry<K, V>* next;

    uint32_t keyLength;

    K* key;
    V value;
};

template <class K, class V>
struct Table {
    uint32_t lanes;
    TableEntry<K, V>** entries;

    Table<K, V>(uint32_t _lanes = 10) { // the default number of 'lanes' of 10 is arbitrary
        lanes = _lanes;
        entries = (TableEntry<K, V>**) calloc(lanes, sizeof (TableEntry<K, V>*));
    }

    void* operator new(size_t size) {
        return (Table<K,V>*) malloc(sizeof (Table<K,V>));
    }

    void operator delete(void* p) {
        //free(p);
    }

    static uint32_t hash(K* key, uint32_t keyLength, uint32_t capacity) {
        uint32_t hash = 0;

        uint32_t i = 0;
        while (i < keyLength) {
            hash += *key;
            hash += (hash << 10);
            hash ^= (hash >> 6);
            key++;
            i++;
        }

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        return hash % capacity;
    }

    signed int insert(K* key, uint32_t keyLength, V value) {
        TableEntry<K, V>* entry = lookup(key, keyLength);

        uint32_t hashValue;

        if (!entry) {
            entry = (TableEntry<K, V>*) malloc(sizeof (TableEntry<K, V>));
            entry->key = key;
            entry->keyLength = keyLength;
            entry->value = value;

            hashValue = hash(key, keyLength, lanes);
            entry->next = entries[hashValue];
            entries[hashValue] = entry;

            return 0;

        } else {
            //free(entry->value);

            entry->value = value;
            return 1;
        }
    }

    TableEntry<K, V>* lookup(K* key, uint32_t keyLength) {
        TableEntry<K, V>* entry = entries[hash(key, keyLength, lanes)];

        for (; entry != NULL; entry = entry->next) {
            if (Str_memeq(key, keyLength, entry->key, entry->keyLength)) {
                return entry;
            }
        }

        return NULL;
    }
};

#endif

