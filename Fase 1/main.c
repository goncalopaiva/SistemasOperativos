/**
 * @author BJGomes
 *
 * Sars-CoV-2 mutation analysis
 * Based on the comparison techniques proposed in:
 * https://github.com/tonygeorge1984/Python-Sars-Cov-2-Mutation-Analysis
 */

#include "main.h"

int main()
{
    //Fase 1 - Requisitos A, B e C
    //fase1(5);

    //Fase 2 - Requisitos A, B, C e D
    fase2(5);
}

/**
 * Implementação dos requisitos A, B, C - Uso de procesos filhos
 * @param N - number of child processes to be created
 */
int fase1(int N)
{
    /* FASE 1
       Implementação dos requisitos A, B e C
    */

    int count = 0;
    long time_usec_begin;
    long time_usec_end;
    long elapsed_time;

    printf("\n----------WITH CHILD PROCESSES--------\n");

    get_time_useconds(&time_usec_begin);

    GENOME_LIST *gl = (GENOME_LIST *)calloc(1, sizeof(GENOME_LIST));

    read_genomes(gl, "input/cds.fna");

    printf("Read: %ld genomes\n", gl->n_genomes);

    get_time_useconds(&time_usec_end);
    elapsed_time = (long)(time_usec_end - time_usec_begin);
    printf("Read time = %ld microseconds\n", elapsed_time);

    MUTATION_ARRAY *mutation_array = (MUTATION_ARRAY *)calloc(1, sizeof(MUTATION_ARRAY));
    GENOME *g = gl->phead;

    long M = gl->n_genomes;

    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
            perror("fork(): Error\n");
        if (pid == 0)
        {

            //Child process
            printf("Creating child process %d/%d (PID: %d)\n", i + 1, N, getpid());

            g = gl->phead;
            g = jump_list(g, N);

            while (g != NULL)
            {
                //string path = result/mutations[i].txt
                char str[4];
                char path[] = "result/mutationsChild";
                char ext[] = ".txt";
                sprintf(str, "%d", i + 1);
                strcat(path, str);
                strcat(path, ext);
                genome_cmp(g, mutation_array);
                save_mutation_array(mutation_array, path, 0);
                free_mutations(mutation_array);
                count++;
                g = jump_list(g, N);
                printf("%li %% completed\n", (((count * N) + (N + gl->n_genomes % N)) * 100) / gl->n_genomes);
            }

            //printf("%li %% completed\n", (((count * N) + (N + gl->n_genomes % N)) * 100) / gl->n_genomes);
            //Cada filho, ao terminar, envia um sinal ao pai e o pai imprime a %

            //signal(SIGUSR2, print_statistics);
            exit(0);
        }
    }
    for (int i = 0; i < N; i++)
    {
        //Parent process
        int status;
        status = 0;
        pid_t childpid = wait(&status);
    }

    get_time_useconds(&time_usec_end);
    elapsed_time = (long)(time_usec_end - time_usec_begin);
    printf("Total time = %ld microseconds\n", elapsed_time);

    return 0;
}

/**
 * Implementação dos requisitos A, B, C e D - Uso de procesos filho e pipes
 * @param N - number of child processes to be created
 */
int fase2(int N)
{
    /* FASE 2
       Implementação dos requisitos A, B, C e D
    */

    int count = 0;
    int fd[2];

    long time_usec_begin;
    long time_usec_end;
    long elapsed_time;

    printf("\n----------WITH CHILD PROCESSES--------\n");
    printf("---------------AND PIPES--------------\n");

    get_time_useconds(&time_usec_begin);

    GENOME_LIST *gl = (GENOME_LIST *)calloc(1, sizeof(GENOME_LIST));

    read_genomes(gl, "input/cds.fna");

    printf("Read: %ld genomes\n", gl->n_genomes);

    get_time_useconds(&time_usec_end);
    elapsed_time = (long)(time_usec_end - time_usec_begin);
    printf("Read time = %ld microseconds\n", elapsed_time);

    MUTATION_ARRAY *mutation_array = (MUTATION_ARRAY *)calloc(1, sizeof(MUTATION_ARRAY));
    GENOME *g = gl->phead;

    long M = gl->n_genomes;
    pipe(fd);

    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
            perror("fork(): Error\n");
        if (pid == 0)
        {
            //Child process
            printf("Creating child process %d/%d (PID: %d)\n", i + 1, N, getpid());

            g = gl->phead;
            g = jump_list(g, N);

            int detail = 0;
            ssize_t n;
            char *buf1 = (char *)malloc(sizeof(char) * 100000000);
            if (fd < 0)
            {
                printf("Error opening pipe\n");
            }

            while (g != NULL)
            {
                close(fd[0]);
                genome_cmp(g, mutation_array);
                for (int i = 0; i < mutation_array->n_mutations; i++)
                {
                    MUTATION *aux = mutation_array->mutations + i;
                    sprintf(buf1, "%s;%s|%s|(%d)", aux->genome_a, aux->genome_b, aux->gene, aux->seq_mutations.n);
                    if (detail)
                    {
                        for (int j = 0; j < aux->seq_mutations.n; j++)
                        {
                            sprintf(buf1, "%s%d;", buf1, aux->seq_mutations.arr[j]);
                        }
                    }
                    strcat(buf1, "\n");
                    //write(STDOUT_FILENO, buf1, strlen(buf1));
                    write(fd[1], buf1, strlen(buf1) + 1);
                }
                free_mutations(mutation_array);
                count++;
                printf("%li %% completed\n", (((count * N) + (N + gl->n_genomes % N)) * 100) / gl->n_genomes);
                g = jump_list(g, N);
            }
            close(fd[1]);
            exit(0);
        }
        else
        {
            int n;
            close(fd[1]);
            char file[50] = "result/Fase2_Output.txt";
            char *buf2 = (char *)malloc(sizeof(char) * 100000000);
            while ((n = read(fd[0], buf2, sizeof(n))) > 0)
            {
                //fprintf(file, "%s", buf2);
                //write(STDOUT_FILENO, buf2, strlen(buf2));
                int fp = open(file, O_CREAT | O_WRONLY | O_APPEND, 700);
                write(fp, buf2, strlen(buf2));
            }
            close(fd[0]);
            //fclose(fp);
            waitpid(-1, NULL, 0);
        }
    }

    get_time_useconds(&time_usec_end);
    elapsed_time = (long)(time_usec_end - time_usec_begin);
    printf("Total time = %ld microseconds\n", elapsed_time);

    return 0;
}

/**
 * Adds a given string to a file
 * @param filename - path of the file
 * @param string - string to be added
 */
void saveToFile(char *filename, char *string)
{
    FILE *fp = fopen(filename, "a");
    fprintf(fp, "%s", string);
    fclose(fp);
}

/**
 * Jumps to a specific node of the linked list
 * @param g - genome 
 * @param k - number of nodes to jump
 * @return genome
 */
GENOME *jump_list(GENOME *g, int k)
{
    while (g != NULL)
    {
        for (int i = 0; i < k; i++)
        {
            g = g->pnext;
        }
        return g;
    }
}

/**
 * Inserts a new genome at the tail of given genome list
 * @param gl - genome list
 * @param g - new genome
 */
void insert_genome(GENOME_LIST *gl, GENOME *g)
{
    g->pnext = NULL;
    g->pprev = NULL;

    if (gl->phead == NULL)
        gl->phead = g;

    if (gl->ptail != NULL)
    {
        g->pprev = gl->ptail;
        gl->ptail->pnext = g;
    }

    gl->ptail = g;
    gl->n_genomes++;
}

/**
 * Searches for a gene in a given genome
 * @param genome - genome to be scanned
 * @param gene_name - gene to searched
 * @return - pointer to the found gene or NULL if no match
 */
GENE *find_gene(GENOME *genome, char *gene_name)
{
    for (int i = 0; i < genome->n_genes; i++)
    {
        if (strcmp((genome->genes + i)->name, gene_name) == 0)
            return genome->genes + i;
    }
    return NULL;
}

/**
 * Inserts a new element into a given int_array
 * @param int_array - given integer array
 * @param element - element to be inserted
 */
void insert_int_array(INT_ARRAY *int_array, int element)
{
    if (int_array->n >= int_array->size)
    {
        int_array->size = (int_array->size != 0) ? int_array->size * 2 : 2;
        int_array->arr = (int *)realloc(int_array->arr, int_array->size * sizeof(int));
    }

    int_array->arr[int_array->n] = element;
    int_array->n++;
}

/**
 * Compares two genes by subtracting each of the nucleotide sequences values of g1 and g2
 * @param g1 - gene 1 to be compared
 * @param g2 - gene 2 to be compared
 * @return integer array containing the differences between the tow genes
 */
INT_ARRAY *gene_cmp(GENE g1, GENE g2)
{
    int i;
    INT_ARRAY *to_return = (INT_ARRAY *)calloc(1, sizeof(INT_ARRAY));

    for (i = 0; *(g1.seq + i) != '\0'; i++)
    {
        int x = abs((int)*(g1.seq + i) - (int)*(g2.seq + i));
        if (x != 0)
            insert_int_array(to_return, i);
    }

    return to_return;
}

/**
 * Inserts a new mutation into a given mutation array
 * @param mutation_array - array of mutations
 * @param genome_a - genome used for comparison against genome b
 * @param genome_b - genome used for comparison against genome a
 * @param gene - gene on which the two genomes were previously compared
 * @param gene_mut - integer array with all the found mutations
 */
void insert_mutation(MUTATION_ARRAY *mutation_array, char *genome_a, char *genome_b, char *gene, INT_ARRAY *gene_mut)
{
    if (mutation_array->n_mutations >= mutation_array->size_mutations)
    {
        mutation_array->size_mutations = (mutation_array->size_mutations != 0) ? mutation_array->size_mutations * 2 : 2;
        mutation_array->mutations = (MUTATION *)realloc(mutation_array->mutations, mutation_array->size_mutations * sizeof(MUTATION));
    }
    MUTATION *aux = mutation_array->mutations + mutation_array->n_mutations;
    strcpy(aux->genome_a, genome_a);
    strcpy(aux->genome_b, genome_b);
    strcpy(aux->gene, gene);

    aux->seq_mutations = *gene_mut;

    mutation_array->n_mutations++;
}

/**
 * Compares a given genome against all its subsequent genemoes in a genome list
 * @param genome - reference genome to compare against all the subsequent genomes
 * @param mutation_array - array in which the comparison results (mutations) will be stored
 */
void genome_cmp(GENOME *genome, MUTATION_ARRAY *mutation_array)
{
    GENE *base_gene;
    INT_ARRAY *gene_mut = NULL;

    for (int i = 0; i < genome->n_genes; i++)
    {
        base_gene = genome->genes + i;

        GENOME *tmp_genome = genome->pnext;
        while (tmp_genome != NULL)
        {
            GENE *new_gene = find_gene(tmp_genome, base_gene->name);
            if (new_gene != NULL)
            {
                if ((gene_mut = gene_cmp(*base_gene, *new_gene)) != NULL)
                {
                    insert_mutation(mutation_array, genome->name, tmp_genome->name, base_gene->name, gene_mut);
                }
            }
            tmp_genome = tmp_genome->pnext;
        }
    }
}

/**
 * Removes white spaces ' ' and '\n' from a given sting
 * @param str - string with no ' ' or '\n'
 */
void remove_white_spaces(char *str)
{
    int c = 0, j = 0;
    while (str[c] != '\0')
    {
        if (str[c] != ' ' && str[c] != '\n')
            str[j++] = str[c];
        c++;
    }
    str[j] = '\0';
}

/**
 * Searches, by name, for a given gene in a known gene dictionary
 * @param name - gene name to search for
 * @return - pointer to the found dictionary entry or NULL if non-existent
 */
GENE_DICT *find_gene_dict(char *name)
{
    for (int i = 0; i < DICT_SIZE; i++)
        if (strcmp(name, gd[i].name) == 0)
            return gd + i;
    return NULL;
}

/**
 * Finds the number of dummy nucleotides to append to the nucleotide sequence
 * Not required but useful if displaying a square matrix with the gene comparison result
 * @param name - gene name
 * @return - number of dummy nucleotides to append
 */
int get_gene_padding(char *name)
{
    GENE_DICT *gene = find_gene_dict(name);
    if (gene != NULL)
        return gene->padding;
    return 0;
}

/**
 * Creates a new gene given a gene name and a nucleotide sequence
 * @param name - new gene name
 * @param seq - new gene nucleotide sequence
 * @return - pointer to the created gene
 */
GENE *create_gene(char *name, char *seq)
{
    GENE *ret = (GENE *)malloc(sizeof(GENE));
    remove_white_spaces(seq);
    int N = get_gene_padding(name);
    ret->seq = (char *)malloc(sizeof(char) * (strlen(seq) + N + 1));

    strcpy(ret->name, name);
    sprintf(ret->seq, "%s%.*s", seq, N, "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");
    return ret;
}

/**
 * Inserts a new gene into a given genome
 * @param genome - pre-existing genome
 * @param gene - gene to be inserted in the given genome
 */
void insert_gene(GENOME *genome, GENE *gene)
{
    if (genome->n_genes >= genome->size_genes)
    {
        genome->size_genes = (genome->size_genes != 0) ? genome->size_genes * 2 : 2;
        genome->genes = (GENE *)realloc(genome->genes, genome->size_genes * sizeof(GENE));
    }

    GENE *g = genome->genes + genome->n_genes;
    *g = *gene;

    genome->n_genes++;
}

void read_genomes(GENOME_LIST *gl, char *path)
{
    long bytes, total = 0, size;

    char *cds = NULL;

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("File open");
        exit(1);
    }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    cds = (char *)malloc(sizeof(char) * (size + 1));
    while ((bytes = read(fd, cds + total, BUF_SIZE)))
        total += bytes;

    close(fd);

    parse_genome(gl, cds);
}

char *find_protein_name(char *protein)
{
    for (int i = 0; i < DICT_SIZE; i++)
    {
        if (strcmp(protein, gd[i].prot) == 0)
            return gd[i].name;
    }
    return "";
}

GENOME *find_genome(GENOME_LIST *gl, char *g_id)
{
    if (gl == NULL)
        return NULL;
    if (gl->phead == NULL || gl->ptail == NULL)
        return NULL;

    GENOME *to_return = gl->ptail;
    while (to_return != NULL)
    {
        if (strcmp(g_id, to_return->name) == 0)
            return to_return;
        to_return = to_return->pprev;
    }

    return NULL;
}

/**
 * Parses a given code region sequence by genomes and genes,
 * populating the received genome list with the loaded values
 * @param gl - pointer to the genome list
 * @param cds - loaded given code region sequence containing all the genomes
 */
void parse_genome(GENOME_LIST *gl, char *cds)
{
    int n = 0;

    char *token;
    char needle[] = ">";
    char genome_id[MAX100], protein[MAX100];

    token = strtok(cds, needle);

    while (token != NULL)
    {
        sscanf(token, "%[^.]%*s%s%*[^\n]%n", genome_id, protein, &n);

        strcpy(protein, find_protein_name(protein));
        if (strcmp(protein, "") != 0)
        {

            GENE *new_gene = create_gene(protein, token + n + 1);

            GENOME *p_genome = find_genome(gl, genome_id);
            if (p_genome == NULL)
            {
                p_genome = (GENOME *)calloc(1, sizeof(GENOME));
                strcpy(p_genome->name, genome_id);
                insert_genome(gl, p_genome);
            }
            insert_gene(p_genome, new_gene);
        }

        token = strtok(NULL, needle);
    }

    free(cds);
}

/**
 * prints a given genome to the std output
 * @param genome - genome to be printed
 */
void print_genome(GENOME genome)
{
    GENE *gene = genome.genes;

    printf("Genome: %s, %d\n", genome.name, genome.n_genes);
    for (int i = 0; i < genome.n_genes; i++)
    {
        printf("\tName: %s\n", gene->name);
        printf("\tSequence: %s\n\n", gene->seq);

        gene++;
    }
}

/**
 * Gets the number of microseconds elapsed since 1/1/1970
 * @param time_usec - variable in which the elapsed time will be stored
 * @return - elapsed time since 1/1/1970
 */
long get_time_useconds(long *time_usec)
{
    struct timeval time;
    gettimeofday(&time, NULL);

    *time_usec = (long)(time.tv_sec * 1000000 + time.tv_usec);
    return *time_usec;
}

/**
 * Saves the mutation array to file
 * @param mutation_array - array containing all the discovered mutations
 * @param path - path to the file on which the results will be stored
 * @param detail - detail flag.
 * (0) outputs only the number of found mutations per genome / gene
 * (1) outputs all the found mutations (on a nucleotide level) per genome / gene
 */
void save_mutation_array(MUTATION_ARRAY *mutation_array, char *path, int detail)
{
    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0744);

    if (fd == -1)
    {
        perror("File open");
        exit(1);
    }

    char *buf = (char *)malloc(sizeof(char) * 1000000);

    for (int i = 0; i < mutation_array->n_mutations; i++)
    {

        MUTATION *aux = mutation_array->mutations + i;
        sprintf(buf, "%s;%s|%s|(%d)", aux->genome_a, aux->genome_b, aux->gene, aux->seq_mutations.n);
        if (detail)
        {
            for (int j = 0; j < aux->seq_mutations.n; j++)
            {
                sprintf(buf, "%s%d;", buf, aux->seq_mutations.arr[j]);
            }
        }

        strcat(buf, "\n");
        write(fd, buf, strlen(buf));
    }

    close(fd);
    free(buf);
}

/**
 * Frees a given mutation array
 * @param mutation_array - pointer to the previously allocated mutation array
 */
void free_mutations(MUTATION_ARRAY *mutation_array)
{
    for (int i = 0; i < mutation_array->n_mutations; i++)
    {
        free((mutation_array->mutations + i)->seq_mutations.arr);
    }

    free(mutation_array->mutations);

    mutation_array->n_mutations = mutation_array->size_mutations = 0;
    mutation_array->mutations = NULL;
}
