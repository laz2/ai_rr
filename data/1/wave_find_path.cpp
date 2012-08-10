// wave_find_path.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @name Graph
/// Структура для представления неориентированного графа
/// в виде матрицы смежности и функции для работы с таким представлением.
/// @{

/// Структура для представления графа в виде матрицы смежности.
/// 
/// @note Работа с вершинами идет по их порядковым номерам.
///
struct Graph
{
	/// Количество вершин.
	int    size;
	/// Массив имен вершин.
	char **names;
	/// Матрицы смежности.
	int  **matrix;
};

/// Считывает граф из текстового файла с путем @p path.
///
/// В файле находиться:
/// 1. Количество вершин
/// 2. Имена вершин, по одной на строку
/// 3. Матрица смежности
///
Graph *load_graph(const char *path)
{
	FILE *in = fopen(path, "r");
	if (!in) {
		printf("Error: Cannot open file '%s'.\n", path);
		exit(1);
	}

	Graph *graph = new Graph;

	// Считываем количество вершин.
	//
	fscanf(in, "%d", &graph->size);

	// Считываем имена вершин.
	//
	graph->names = new char*[graph->size];
	for (int i = 0; i < graph->size; ++i) {
		char buf[60];
		fscanf(in, "%s", buf);
		graph->names[i] = strdup(buf);
	}

	// Создаем матрицу смежности.
	//
	graph->matrix = new int*[graph->size];
	for (int i = 0; i < graph->size; ++i) {
		graph->matrix[i] = new int[graph->size];
	}

	// Считываем матрицу смежности.
	//
	for (int i = 0; i < graph->size; ++i) {
		for (int j = 0; j < graph->size; ++j) {
			fscanf(in, "%d", &graph->matrix[i][j]);
		}
	}

	return graph;
}

/// Освобождает память, выделенную под граф @p graph.
///
void delete_graph(Graph *graph)
{
	for (int i = 0; i < graph->size; ++i) {
		delete[] graph->names[i];
		delete[] graph->matrix[i];
	}
	delete[] graph->names;
	delete[] graph->matrix;
}

/// Поиск в графе @p graph порядкового номера вершины с именем @p name.
///
int find_vertex(Graph *graph, const char *name)
{
	for (int i = 0; i < graph->size; ++i)
		if (!strcmp(graph->names[i], name))
			return i;
	return -1;
}

/// Вывод на консоль @p count пробелов.
///
void print_spaces(int count)
{
	for (int i = 0; i < count; ++i)
		printf(" ");
}

/// Выводит на консоль граф @p graph.
///
void print_graph(const Graph *graph)
{
	// Определим максимальную длину имени вершины.
	//
	int max_name_len = 0;
	for (int i = 0; i < graph->size; ++i) {
		int name_len = strlen(graph->names[i]);
		if (name_len > max_name_len)
			max_name_len = name_len;
	}

	// Выведем имена вершин в заголовке матрицы смежности.
	//
	print_spaces(max_name_len + 1);
	for (int i = 0; i < graph->size; ++i)
		printf("%s ", graph->names[i]);
	printf("\n");

	// Вывод строк матрицы смежности.
	//
	for (int i = 0; i < graph->size; ++i) {
		printf("%s", graph->names[i]);
		print_spaces(max_name_len - strlen(graph->names[i]) + 1);

		for (int j = 0; j < graph->size; ++j) {
			printf("%d", graph->matrix[i][j]);
			print_spaces(strlen(graph->names[j]));
		}

		printf("\n");
	}
}

/// @}

/// @name FindMinPath
/// Функции и структуры для работы волного
/// алгоритма поиска минимального пути.
///
/// @see find_min_path
///
/// @{
///

/// Стуруктура для представления волны.
///
struct Wave
{
	/// Количество вершин в волне.
	int  size;
	/// Массив вершин волны.
	int *vertexes;
};

/// Структура для представления элемента двусвязного списка волн.
///
struct WavesListEntry
{
	/// Волна.
	Wave           *wave;
	/// Предыдущий элемент списка волн.
	WavesListEntry *prev;
	/// Следующий элемент списка волн.
	WavesListEntry *next;
};

/// Создает и инициализирует элемент списка волн.
///
inline WavesListEntry *list_new()
{
	WavesListEntry *list = new WavesListEntry;
	list->prev = 0;
	list->next = 0;
	return list;
}

/// @brief Создает следующую волну из непроверенных вершин,
///        смежных с вершинами из волны @p wave неориентированного графа @p graph.
///
/// @param graph                обрабатываемый неориентированный граф.
/// @param wave                 текущая волна.
/// @param not_checked_vertexes множество непроверенных вершин.
///
/// @return созданная новая волна.
Wave *create_wave(Graph *graph, Wave *wave, bool *not_checked_vertexes)
{
	// Создаем новую волну пустой.
	//
	Wave *new_wave = new Wave;
	new_wave->size = 0;
	new_wave->vertexes = 0;

	// Для обеспечения возможности представления вершин волны в виде массив
	// будем использовать двухпроходной алгоритм построения волны.
	//

	// Определим верхнюю границу количества вершин в создаваемой волне.
	// В новую волну будут входить только те вершины, которые
	// смежны с вершинами из предыдущей волны @p wave и являются непроверенными.
	//
	int max_wave_size = 0;
	for (int i = 0; i < wave->size; ++i) {
		for (int j = 0; j < graph->size; ++j) {
			if (graph->matrix[wave->vertexes[i]][j] && not_checked_vertexes[j])
				++max_wave_size;
		}
	}

	// Если волна new_wave непустая, то начнем ее формирование.
	//
	if (max_wave_size) {
		new_wave->vertexes = new int[max_wave_size];

		int new_wave_idx = 0;
		for (int i = 0; i < wave->size; ++i) {
			for (int j = 0; j < graph->size; ++j) {
				if (graph->matrix[wave->vertexes[i]][j] && not_checked_vertexes[j]) {
					new_wave->vertexes[new_wave_idx++] = j;
					++new_wave->size;
					not_checked_vertexes[j] = false;
				}
			}
		}
	}

	return new_wave;
}

/// @brief Удаляет все волны из списка и список волн.
///
/// @param waves_list_head начало списка волн.
///
void erase_waves_list(WavesListEntry *head)
{
	WavesListEntry *curr = head;
	while (curr) {
		if (curr->wave->size)
			delete[] curr->wave->vertexes;
		delete curr->wave;

		WavesListEntry *next = curr->next;
		delete curr;

		curr = next;
	}
}

/// @brief Проверяет, входит ли вершина с номером @p vertex в
///        волну @p wave.
///
/// @param vertex номер вершины.
/// @param wave   волна.
///
/// @return true, если вершина входит в волну, иначе false.
///
bool is_in_wave(int vertex, Wave *wave)
{
	for (int i = 0; i < wave->size; ++i)
		if (wave->vertexes[i] == vertex)
			return true;
	return false;
}

/// @brief Выводит на консоль вершины волны.
///
/// @param graph неориентированный граф.
/// @param wave  волна для вывода.
///
void print_wave(Graph *graph, Wave *wave)
{
	if (wave->size) {
		printf("\tWave: ");
		printf("%s", graph->names[wave->vertexes[0]]);
		for (int i = 1; i < wave->size; ++i)
			printf(", %s", graph->names[wave->vertexes[i]]);
		printf("\n");
	} else {
		printf("\tWave is empty\n");
	}
}

/// @brief Ищет для @p vertex любую смежную вершину из волны @p prev_wave в графе @p graph.
///
/// @param graph     обрабатываемый неориентированный граф.
/// @param vertex    вершина, для которой ищется смежная вершина из @p prev_wave.
/// @param prev_wave предыдущая волна, из которой ищется смежная вершина.
///
/// @return номер найденной вершине.
///
int find_prev_adj_vertex(Graph *graph, int vertex, Wave *prev_wave)
{
	for (int i = 0; i < prev_wave->size; ++i)
		if (graph->matrix[prev_wave->vertexes[i]][vertex])
			return prev_wave->vertexes[i];

	return -1;
}

/// Выводит на консоль множество непроверенных вершин.
///
/// @param graph                неориентированный граф.
/// @param not_checked_vertexes непроверенные вершины.
///
void print_not_checked_vertexes(Graph *graph, bool *not_checked_vertexes)
{
	printf("\tNot checked: ");

	bool first = true;
	for (int i = 0; i < graph->size; ++i) {
		if (not_checked_vertexes[i]) {
			if (first) {
				first = false;
				printf("%s", graph->names[i]);
			} else {
				printf(", %s", graph->names[i]);
			}
		}
	}

	printf("\n");
}

/// Выводит на консоль вершины пути.
///
/// @param graph         неориентированный граф.
/// @param path_size     количество вершин в пути.
/// @param path_vertexes массив вершин пути.
///
void print_path(Graph *graph, int path_size, int *path_vertexes)
{
	printf("Path: %s", graph->names[path_vertexes[0]]);
	for (int i = 1; i < path_size; ++i)
		printf(" -> %s", graph->names[path_vertexes[i]]);
	printf("\n");
}

/// Находит один из минимальных путей в графе @p graph
/// от вершины @p beg_vertex до вершины @p end_vertex.
///
/// @note Найденный путь выводиться на консоль.
///
/// Волновой алгоритм поиска одного из минимальных путей:
/// 1. Добавить все вершины графа (кроме начальной вершины пути) в множество непроверенных вершин.
///
/// 2. Создать начальную волну и добавить в нее начальную вершину пути.
///
/// 3. Начальная волна - новая волна. Новой волной будем называть последнюю созданную волну.
///
/// 4. Сформировать следующую волну для новой волны. В нее попадет та вершина,
///    которая является смежной вершине из новой волны и присутствует во множестве непроверенных вершин.
///    Если вершина попала в формируемую волну, то ее надо исключить из множества непроверенных вершин.
///    Созданную волну установим как следующую для новой волны, и после этого созданную волну будем считать новой волной.
///
/// 5. Если новая волна пуста, то значит между вершинами не существует пути.
///    Завершить алгоритм.
///
/// 6. Если в текущей волне есть конечная вершина, то перейти к пункту 7, иначе к пункту 4.
///
/// 7. Сформировать один из минимальных путей, проходя в обратном порядке по списку волн.
///
/// @param graph      неориентированный граф, в котором будет находится минимальный путь.
/// @param beg_vertex номер начальной вершины пути.
/// @param end_vertex номер конечной вершины пути.
///
void find_min_path(Graph *graph, int beg_vertex, int end_vertex)
{
	// 1. Добавить все вершины графа (кроме начальной вершины пути) в множество непроверенных вершин.
	//
	bool *not_checked_vertexes = new bool[graph->size];

	for (int i = 0; i < graph->size; ++i)
		not_checked_vertexes[i] = true;
	not_checked_vertexes[beg_vertex] = false;

	// 2. Создать начальную волну и добавить в нее начальную вершину пути.
	// 3. Начальная волна - новая волна.
	//
	Wave *new_wave = new Wave;
	new_wave->size = 1;
	new_wave->vertexes = new int[1];
	new_wave->vertexes[0] = beg_vertex;

	// Создаем начало списка волн.
	//
	WavesListEntry *waves_list_head = list_new();
	waves_list_head->wave = new_wave;
	WavesListEntry *waves_list_tail = waves_list_head;

	// Выводим на консоль информацию по первому шагу создания списка волн.
	//
	int step_num = 1;
	printf("Step %d:\n", step_num);
	print_not_checked_vertexes(graph, not_checked_vertexes);
	print_wave(graph, new_wave);

	// Начинаем цикл для формирования волн.
	//
	do {
		// 4. Сформировать следующую волну для новой волны.
		//    Установить созданную волну как следующую для новой волны.
		//    Созданная волна - новая волна.
		//
		new_wave = create_wave(graph, new_wave, not_checked_vertexes);

		// Выводим на консоль информацию по текущему шагу создания списка волн.
		//
		++step_num;
		printf("Step %d:\n", step_num);
		print_not_checked_vertexes(graph, not_checked_vertexes);
		print_wave(graph, new_wave);

		WavesListEntry *waves_list_curr = list_new();
		waves_list_curr->wave = new_wave;
		waves_list_tail->next = waves_list_curr;
		waves_list_curr->prev = waves_list_tail;

		waves_list_tail = waves_list_curr;

		// 5. Если новая волна пуста, то значит между вершинами не существует пути.
		//
		if (!new_wave->size) {
			erase_waves_list(waves_list_head);
			delete[] not_checked_vertexes;
			printf("Path isn't exist.\n");
			return;
		}

		// 6. Если в текущей волне есть конечная вершина, то перейти к пункту 7, иначе к пункту 4.
		//
	} while (!is_in_wave(end_vertex, new_wave));

	delete[] not_checked_vertexes;

	int curr_vertex = end_vertex;
	int curr_idx = step_num - 1;

	// Создаем массив вершин пути.
	// Последним элементом в массиве устанавливаем номер конечной вершины пути.
	//
	int *path_vertexes = new int[step_num];
	path_vertexes[curr_idx] = end_vertex;

	// Начинаем цикл с конца списка волн в начало.
	//
	WavesListEntry *waves_list_curr = waves_list_tail->prev;
	for (; waves_list_curr; waves_list_curr = waves_list_curr->prev) {
		curr_vertex = find_prev_adj_vertex(graph, curr_vertex, waves_list_curr->wave);
		path_vertexes[--curr_idx] = curr_vertex;
	}

	print_path(graph, step_num, path_vertexes);
	//delete[] path_vertexes;

	erase_waves_list(waves_list_head);
}

/// @}

/// Подготавливает запуск и запускает тестовый пример для алгоритма поиска минимального пути.
///
/// @param number     порядковый номер тестового примера.
/// @param graph_file путь к файлу с тестовым графом для функции #load_graph.
/// @param beg_name   имя начальной вершина для поиска минимального пути.
/// @param end_name   имя конечной вершина для поиска минимального пути.
///
/// @see load_graph
/// @see find_min_path
///
void run_testcase(int number, const char *graph_file, const char *beg_name, const char *end_name)
{
	printf("\n\n[Testcase %d]\n", number);

	// Считаем граф из файла.
	//
	Graph *graph = load_graph(graph_file);

	// Распечатаем граф на консоль.
	//
	printf("Graph: \n");
	print_graph(graph);

	// Найдем начальную вершину для тестового поиска минимального пути.
	//
	int beg = find_vertex(graph, beg_name);
	if (beg == -1) {
		printf("Vertex with name %s not found in graph.\n", beg_name);
		exit(1);
	}

	// Найдем конечную вершину для тестового поиска миниального пути.
	//
	int end = find_vertex(graph, end_name);
	if (end == -1) {
		printf("Vertex with name %s not found in graph.\n", end_name);
		exit(1);
	}

	printf("Find minimal path from '%s' to '%s'\n", beg_name, end_name);

	// Найдем минимальный пути между начальной и конечной вершинами.
	//
	find_min_path(graph, beg, end);

	// Не забудем освободить память, которую занимает граф.
	//
	delete_graph(graph);
}

int _tmain(int argc, _TCHAR* argv[])
{
	run_testcase(1, "graph1.txt", "A", "C");
	run_testcase(2, "graph2.txt", "A", "F");
	run_testcase(3, "graph3.txt", "A", "K");
	run_testcase(4, "graph4.txt", "V5", "V11");
	run_testcase(5, "graph5.txt", "V1", "V9");

	return 0;
}