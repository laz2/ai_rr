// wave_find_path.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @name Graph
/// ��������� ��� ������������� ������������������ �����
/// � ���� ������� ��������� � ������� ��� ������ � ����� ��������������.
/// @{

/// ��������� ��� ������������� ����� � ���� ������� ���������.
/// 
/// @note ������ � ��������� ���� �� �� ���������� �������.
///
struct Graph
{
	/// ���������� ������.
	int    size;
	/// ������ ���� ������.
	char **names;
	/// ������� ���������.
	int  **matrix;
};

/// ��������� ���� �� ���������� ����� � ����� @p path.
///
/// � ����� ����������:
/// 1. ���������� ������
/// 2. ����� ������, �� ����� �� ������
/// 3. ������� ���������
///
Graph *load_graph(const char *path)
{
	FILE *in = fopen(path, "r");
	if (!in) {
		printf("Error: Cannot open file '%s'.\n", path);
		exit(1);
	}

	Graph *graph = new Graph;

	// ��������� ���������� ������.
	//
	fscanf(in, "%d", &graph->size);

	// ��������� ����� ������.
	//
	graph->names = new char*[graph->size];
	for (int i = 0; i < graph->size; ++i) {
		char buf[60];
		fscanf(in, "%s", buf);
		graph->names[i] = strdup(buf);
	}

	// ������� ������� ���������.
	//
	graph->matrix = new int*[graph->size];
	for (int i = 0; i < graph->size; ++i) {
		graph->matrix[i] = new int[graph->size];
	}

	// ��������� ������� ���������.
	//
	for (int i = 0; i < graph->size; ++i) {
		for (int j = 0; j < graph->size; ++j) {
			fscanf(in, "%d", &graph->matrix[i][j]);
		}
	}

	return graph;
}

/// ����������� ������, ���������� ��� ���� @p graph.
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

/// ����� � ����� @p graph ����������� ������ ������� � ������ @p name.
///
int find_vertex(Graph *graph, const char *name)
{
	for (int i = 0; i < graph->size; ++i)
		if (!strcmp(graph->names[i], name))
			return i;
	return -1;
}

/// ����� �� ������� @p count ��������.
///
void print_spaces(int count)
{
	for (int i = 0; i < count; ++i)
		printf(" ");
}

/// ������� �� ������� ���� @p graph.
///
void print_graph(const Graph *graph)
{
	// ��������� ������������ ����� ����� �������.
	//
	int max_name_len = 0;
	for (int i = 0; i < graph->size; ++i) {
		int name_len = strlen(graph->names[i]);
		if (name_len > max_name_len)
			max_name_len = name_len;
	}

	// ������� ����� ������ � ��������� ������� ���������.
	//
	print_spaces(max_name_len + 1);
	for (int i = 0; i < graph->size; ++i)
		printf("%s ", graph->names[i]);
	printf("\n");

	// ����� ����� ������� ���������.
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
/// ������� � ��������� ��� ������ �������
/// ��������� ������ ������������ ����.
///
/// @see find_min_path
///
/// @{
///

/// ���������� ��� ������������� �����.
///
struct Wave
{
	/// ���������� ������ � �����.
	int  size;
	/// ������ ������ �����.
	int *vertexes;
};

/// ��������� ��� ������������� �������� ����������� ������ ����.
///
struct WavesListEntry
{
	/// �����.
	Wave           *wave;
	/// ���������� ������� ������ ����.
	WavesListEntry *prev;
	/// ��������� ������� ������ ����.
	WavesListEntry *next;
};

/// ������� � �������������� ������� ������ ����.
///
inline WavesListEntry *list_new()
{
	WavesListEntry *list = new WavesListEntry;
	list->prev = 0;
	list->next = 0;
	return list;
}

/// @brief ������� ��������� ����� �� ������������� ������,
///        ������� � ��������� �� ����� @p wave ������������������ ����� @p graph.
///
/// @param graph                �������������� ����������������� ����.
/// @param wave                 ������� �����.
/// @param not_checked_vertexes ��������� ������������� ������.
///
/// @return ��������� ����� �����.
Wave *create_wave(Graph *graph, Wave *wave, bool *not_checked_vertexes)
{
	// ������� ����� ����� ������.
	//
	Wave *new_wave = new Wave;
	new_wave->size = 0;
	new_wave->vertexes = 0;

	// ��� ����������� ����������� ������������� ������ ����� � ���� ������
	// ����� ������������ ������������� �������� ���������� �����.
	//

	// ��������� ������� ������� ���������� ������ � ����������� �����.
	// � ����� ����� ����� ������� ������ �� �������, �������
	// ������ � ��������� �� ���������� ����� @p wave � �������� ��������������.
	//
	int max_wave_size = 0;
	for (int i = 0; i < wave->size; ++i) {
		for (int j = 0; j < graph->size; ++j) {
			if (graph->matrix[wave->vertexes[i]][j] && not_checked_vertexes[j])
				++max_wave_size;
		}
	}

	// ���� ����� new_wave ��������, �� ������ �� ������������.
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

/// @brief ������� ��� ����� �� ������ � ������ ����.
///
/// @param waves_list_head ������ ������ ����.
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

/// @brief ���������, ������ �� ������� � ������� @p vertex �
///        ����� @p wave.
///
/// @param vertex ����� �������.
/// @param wave   �����.
///
/// @return true, ���� ������� ������ � �����, ����� false.
///
bool is_in_wave(int vertex, Wave *wave)
{
	for (int i = 0; i < wave->size; ++i)
		if (wave->vertexes[i] == vertex)
			return true;
	return false;
}

/// @brief ������� �� ������� ������� �����.
///
/// @param graph ����������������� ����.
/// @param wave  ����� ��� ������.
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

/// @brief ���� ��� @p vertex ����� ������� ������� �� ����� @p prev_wave � ����� @p graph.
///
/// @param graph     �������������� ����������������� ����.
/// @param vertex    �������, ��� ������� ������ ������� ������� �� @p prev_wave.
/// @param prev_wave ���������� �����, �� ������� ������ ������� �������.
///
/// @return ����� ��������� �������.
///
int find_prev_adj_vertex(Graph *graph, int vertex, Wave *prev_wave)
{
	for (int i = 0; i < prev_wave->size; ++i)
		if (graph->matrix[prev_wave->vertexes[i]][vertex])
			return prev_wave->vertexes[i];

	return -1;
}

/// ������� �� ������� ��������� ������������� ������.
///
/// @param graph                ����������������� ����.
/// @param not_checked_vertexes ������������� �������.
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

/// ������� �� ������� ������� ����.
///
/// @param graph         ����������������� ����.
/// @param path_size     ���������� ������ � ����.
/// @param path_vertexes ������ ������ ����.
///
void print_path(Graph *graph, int path_size, int *path_vertexes)
{
	printf("Path: %s", graph->names[path_vertexes[0]]);
	for (int i = 1; i < path_size; ++i)
		printf(" -> %s", graph->names[path_vertexes[i]]);
	printf("\n");
}

/// ������� ���� �� ����������� ����� � ����� @p graph
/// �� ������� @p beg_vertex �� ������� @p end_vertex.
///
/// @note ��������� ���� ���������� �� �������.
///
/// �������� �������� ������ ������ �� ����������� �����:
/// 1. �������� ��� ������� ����� (����� ��������� ������� ����) � ��������� ������������� ������.
///
/// 2. ������� ��������� ����� � �������� � ��� ��������� ������� ����.
///
/// 3. ��������� ����� - ����� �����. ����� ������ ����� �������� ��������� ��������� �����.
///
/// 4. ������������ ��������� ����� ��� ����� �����. � ��� ������� �� �������,
///    ������� �������� ������� ������� �� ����� ����� � ������������ �� ��������� ������������� ������.
///    ���� ������� ������ � ����������� �����, �� �� ���� ��������� �� ��������� ������������� ������.
///    ��������� ����� ��������� ��� ��������� ��� ����� �����, � ����� ����� ��������� ����� ����� ������� ����� ������.
///
/// 5. ���� ����� ����� �����, �� ������ ����� ��������� �� ���������� ����.
///    ��������� ��������.
///
/// 6. ���� � ������� ����� ���� �������� �������, �� ������� � ������ 7, ����� � ������ 4.
///
/// 7. ������������ ���� �� ����������� �����, ������� � �������� ������� �� ������ ����.
///
/// @param graph      ����������������� ����, � ������� ����� ��������� ����������� ����.
/// @param beg_vertex ����� ��������� ������� ����.
/// @param end_vertex ����� �������� ������� ����.
///
void find_min_path(Graph *graph, int beg_vertex, int end_vertex)
{
	// 1. �������� ��� ������� ����� (����� ��������� ������� ����) � ��������� ������������� ������.
	//
	bool *not_checked_vertexes = new bool[graph->size];

	for (int i = 0; i < graph->size; ++i)
		not_checked_vertexes[i] = true;
	not_checked_vertexes[beg_vertex] = false;

	// 2. ������� ��������� ����� � �������� � ��� ��������� ������� ����.
	// 3. ��������� ����� - ����� �����.
	//
	Wave *new_wave = new Wave;
	new_wave->size = 1;
	new_wave->vertexes = new int[1];
	new_wave->vertexes[0] = beg_vertex;

	// ������� ������ ������ ����.
	//
	WavesListEntry *waves_list_head = list_new();
	waves_list_head->wave = new_wave;
	WavesListEntry *waves_list_tail = waves_list_head;

	// ������� �� ������� ���������� �� ������� ���� �������� ������ ����.
	//
	int step_num = 1;
	printf("Step %d:\n", step_num);
	print_not_checked_vertexes(graph, not_checked_vertexes);
	print_wave(graph, new_wave);

	// �������� ���� ��� ������������ ����.
	//
	do {
		// 4. ������������ ��������� ����� ��� ����� �����.
		//    ���������� ��������� ����� ��� ��������� ��� ����� �����.
		//    ��������� ����� - ����� �����.
		//
		new_wave = create_wave(graph, new_wave, not_checked_vertexes);

		// ������� �� ������� ���������� �� �������� ���� �������� ������ ����.
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

		// 5. ���� ����� ����� �����, �� ������ ����� ��������� �� ���������� ����.
		//
		if (!new_wave->size) {
			erase_waves_list(waves_list_head);
			delete[] not_checked_vertexes;
			printf("Path isn't exist.\n");
			return;
		}

		// 6. ���� � ������� ����� ���� �������� �������, �� ������� � ������ 7, ����� � ������ 4.
		//
	} while (!is_in_wave(end_vertex, new_wave));

	delete[] not_checked_vertexes;

	int curr_vertex = end_vertex;
	int curr_idx = step_num - 1;

	// ������� ������ ������ ����.
	// ��������� ��������� � ������� ������������� ����� �������� ������� ����.
	//
	int *path_vertexes = new int[step_num];
	path_vertexes[curr_idx] = end_vertex;

	// �������� ���� � ����� ������ ���� � ������.
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

/// �������������� ������ � ��������� �������� ������ ��� ��������� ������ ������������ ����.
///
/// @param number     ���������� ����� ��������� �������.
/// @param graph_file ���� � ����� � �������� ������ ��� ������� #load_graph.
/// @param beg_name   ��� ��������� ������� ��� ������ ������������ ����.
/// @param end_name   ��� �������� ������� ��� ������ ������������ ����.
///
/// @see load_graph
/// @see find_min_path
///
void run_testcase(int number, const char *graph_file, const char *beg_name, const char *end_name)
{
	printf("\n\n[Testcase %d]\n", number);

	// ������� ���� �� �����.
	//
	Graph *graph = load_graph(graph_file);

	// ����������� ���� �� �������.
	//
	printf("Graph: \n");
	print_graph(graph);

	// ������ ��������� ������� ��� ��������� ������ ������������ ����.
	//
	int beg = find_vertex(graph, beg_name);
	if (beg == -1) {
		printf("Vertex with name %s not found in graph.\n", beg_name);
		exit(1);
	}

	// ������ �������� ������� ��� ��������� ������ ����������� ����.
	//
	int end = find_vertex(graph, end_name);
	if (end == -1) {
		printf("Vertex with name %s not found in graph.\n", end_name);
		exit(1);
	}

	printf("Find minimal path from '%s' to '%s'\n", beg_name, end_name);

	// ������ ����������� ���� ����� ��������� � �������� ���������.
	//
	find_min_path(graph, beg, end);

	// �� ������� ���������� ������, ������� �������� ����.
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