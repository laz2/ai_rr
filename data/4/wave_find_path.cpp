/*
-----------------------------------------------------------------------------
This source file is part of OSTIS (Open Semantic Technology for Intelligent Systems)
For the latest info, see http://www.ostis.net

Copyright (c) 2010 OSTIS

OSTIS is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OSTIS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OSTIS.  If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------
*/

/// @file wave_find_path.cpp
///
/// @brief Программа поиска одного из минимальных путей в неориентированном графе
///        с использованием sc-памяти.
///
/// @author Лазуркин Дмитрий Александрович
/// @author Уваров Константин Александрович
///

#include <assert.h>

#include <libsc.h>

#include <pm_keynodes.h>
#include <pm_utils.h>
#include <segment_utils.h>
#include <sc_generator.h>
#include <sc_tuple.h>
#include <sc_set.h>
#include <sc_relation.h>
#include <sc_list.h>

#include <iostream>

/// Пространство имен ключевых узлов по теории графов.
///
/// Для начала работы необходимо вызвать функцию graph_theory::init,
/// передав в качестве параметра системную сессию.
/// После этого, например, чтобы обратиться к узлу вершина_,
/// достаточно написать graph_keynodes::vertex_.
namespace graph_theory
{
	/// URI сегмента ключевых узлов.
	const char *segment_uri = "/graph_theory/keynode";

	/// Созданный сегмент ключевых узлов.
	sc_segment *segment;

	/// Массив идентификаторов ключевых узлов.
	const char *idtfs[] = {
		"graph structure",             // графовая структура
		"vertex_",                     // вершина_
		"connective_",                 // связка_

		"directed graph structure",    // графовая структура с ориентированными связками
		"directed connective_",        // ориентированная связка_

		"undirected graph structure",  // графовая структура с неориентированными связками
		"undirected connective_",      // неориентированная связка_

		"hypergraph",                  // гиперграф
		"hyperconnective_",            // гиперсвязка_
		"hyperedge_",                  // гиперребро_
		"hyperarc_",                   // гипердуга_

		"pseudograph",                 // псевдограф
		"binary connective_",          // бинарная связка_
		"edge_",                       // ребро_
		"arc_",                        // дуга_
		"loop_",                       // петля_

		"multigraph",                  // мультиграф

		"graph",                       // граф

		"undirected graph",            // неориентированный граф

		"directed graph",              // ориентированный граф

		"route*",                      // маршрут*

		"trail*",                      // цепь*

		"simple trail*"                // простая цепь*
	};

	/// Массив sc-адресов созданных ключевых узлов.
	sc_addr keynodes[sizeof(uris) / sizeof(const char*)];

	/// Ссылки на ключевые узлы для более удобной работы.
	/// @{
	const sc_addr &graph_structure               = keynodes[0];
	const sc_addr &vertex_                       = keynodes[1];
	const sc_addr &connective_                   = keynodes[2];

	const sc_addr &directed_graph_structure      = keynodes[3];
	const sc_addr &directed_connective_          = keynodes[4];

	const sc_addr &undirected_graph_structure    = keynodes[5];
	const sc_addr &undirected_connective_        = keynodes[6];

	const sc_addr &hypergraph                    = keynodes[7];
	const sc_addr &hyperconnective_              = keynodes[8];
	const sc_addr &hyperedge_                    = keynodes[9];
	const sc_addr &hyperarc_                     = keynodes[10];

	const sc_addr &pseudograph                   = keynodes[11];
	const sc_addr &binary_connective_            = keynodes[12];
	const sc_addr &edge_                         = keynodes[13];
	const sc_addr &arc_                          = keynodes[14];
	const sc_addr &loop_                         = keynodes[15];

	const sc_addr &multigraph                    = keynodes[16];

	const sc_addr &graph                         = keynodes[17];

	const sc_addr &undirected_graph              = keynodes[18];

	const sc_addr &directed_graph                = keynodes[19];

	const sc_addr &route                         = keynodes[20];

	const sc_addr &trail                         = keynodes[21];

	const sc_addr &simple_trail                  = keynodes[22];
    /// @}

	/// Производит создание ключевых узлов при помощи сессии @p s
	/// и готовит их к работе.
	void init(sc_session *s)
	{
		// Сперва создадим сегмент /graph_theory/keynode для
		// ключевых узлов.
		segment = create_segment_full_path(s, segment_uri);

		// Пробежимся по массиву идентификаторов ключевых узлов idtfs
		// и создадим каждый ключевой узел.
		// Создаваемые узлы будем заносить в массив keynodes.
		for (int i = 0; i < sizeof(keynodes) / sizeof(sc_addr); ++i) {
			keynodes[i] = s->create_el(segment, SC_N_CONST);
			s->set_idtf(keynodes[i], idtfs[i]);
		}
	}
}

/// @brief Генерирует в sc-памяти неориентированный граф по строке @p str.
///
/// Входная строка @p str должна иметь следующую структуру:
/// "<{vertex_id1, vertex_id2,...},{{vertex_id1, vertex_id2},...}>"
/// Как видно из строки, граф задается классическим способом через множество вершин и множество ребер.
/// Например, "<{A,B,C,D,E,F,G},{{A,B},{C,A},{E,C},{C,D},{E,B},{E,F}}>".
///
/// @param s   sc-сессия, при помощи которой будет производиться работа с sc-памятью.
/// @param seg sc-сегмент, в котором будет сгенерирован неориентированный граф.
/// @param str строка, описывающая неориентированный граф.
///
/// @return сгенерированный неориентированный граф или 0, если входная строка имеет неправильную структуру.
sc_addr gen_undirected_graph(sc_session *s, sc_segment *seg, const sc_string &str)
{
	assert(s);
	assert(seg);

	//
	// Разбор входной строки str, описывающей создаваемый граф.
	//
	enum {FIRST, RNODES, RNODE, RARCS, RARC} state = FIRST;
	sc_string buf;
	typedef std::set<sc_string> vertex_set;
	vertex_set vertexes;
	typedef std::set<std::pair<sc_string, sc_string> > edge_set;
	edge_set edges;
	std::pair<sc_string, sc_string> edge;
#define PUT_NODE() {vertexes.insert(buf); buf.erase();}
#define PUT_ARC() {edge.second = buf; buf.erase(); edges.insert(edge);}
#define PUT_ARC_BEG() {edge.first = buf; buf.erase();}
	for (size_t i = 1; i < str.size() - 1; i++) {
		switch (str[i]) {
			case '{':
				if (state == FIRST) {
					state = RNODE;
					break;
				}
				if(state == RNODES) {
					state = RARCS;
					break;
				}
				if(state == RARCS) {
					state = RARC;
					break;
				}
			break;
			case '}':
				if(state == RNODE) {
					PUT_NODE();
					state = RNODES;
					break;
				}
				if(state == RARC) {
					PUT_ARC();
					state = RARCS;
					break;
				}
			break;
			case ',':
				if(state == RNODE) {
					PUT_NODE();
					break;
				}
				if(state == RARC) {
					PUT_ARC_BEG();
					break;
				}
			break;
			default:
				buf += str[i];
		}
	}
#undef PUT_NODE
#undef PUT_ARC_BEG
#undef PUT_ARC

	//
	// Начинаем генерацию графа в sc-памяти
	//

	// Отображение, которое позволяет переходить быстро от имени вершины к sc-адресу,
	// который обозначает данную вершину графа в sc-памяти
	std::map<sc_string, sc_addr> vertexes_map;

	sc_addr graph = s->create_el(seg, SC_N_CONST); // Переменная будет хранить sc-адрес знака сгенерированного графа

	s->gen3_f_a_f(graph_theory::undirected_graph, 0, seg, SC_A_CONST|SC_POS, graph);

	// Генерация всех узлов графа в sc-памяти.
	// Пробегаем по всем вершинам графа из STL-множества vertexes
	// и добавляем их в генерируемый граф с атрибутом "vertex_".
	for(vertex_set::iterator it = vertexes.begin(); it != vertexes.end(); ++it) {
		sc_addr vertex = s->create_el(seg, SC_N_CONST);
		s->set_idtf(vertex, *it);

		s->gen5_f_a_f_a_f(graph, 0, seg, SC_A_CONST|SC_POS, vertex, 0, seg, SC_A_CONST|SC_POS, graph_theory::vertex_);

		vertexes_map[*it] = vertex;
	}

	// Генерация всех дуг графа в sc-памяти.
	// Пробегаем по всем ребрам графа из STL-множества edges
	// и добавляем их в генерируемый граф с атрибутом "edge_".
	for(edge_set::iterator it = edges.begin(); it != edges.end(); ++it) {
		sc_addr edge = s->create_el(seg, SC_N_CONST);

		s->gen5_f_a_f_a_f(graph, 0, seg, SC_A_CONST|SC_POS, edge, 0, seg, SC_A_CONST|SC_POS, graph_theory::edge_);

		s->gen3_f_a_f(edge, 0, seg, SC_A_CONST|SC_POS, vertexes_map[it->first]);
		s->gen3_f_a_f(edge, 0, seg, SC_A_CONST|SC_POS, vertexes_map[it->second]);
	}

	// Возвращаем из функции sc-адрес сгенерированного графа
	return graph;
}

/// Возвращает в @p v1 и @p v2 вершины, инцидетные ребру @p edge.
void get_edge_vertexes(sc_session *s, sc_addr edge, sc_addr &v1, sc_addr &v2)
{
	assert(s);
	assert(edge);

	sc_iterator *it = s->create_iterator(
		sc_constraint_new(
			CONSTR_3_f_a_a,
			edge,
			SC_A_CONST|SC_POS,
			0,
	), true);

	v1 = it->value(2);
	it->next();
	v2 = it->value(2);

	delete it;
}

/// Выводит на консоль неориентированный граф @p graph.
///
/// @param s     сессия для работы с sc-памятью.
/// @param graph неориентированный граф для вывода на консоль.
void print_graph(sc_session *s, sc_addr graph)
{
	assert(s);
	assert(graph);

	std::set<sc_addr> printed_vertexes; // множество выведенных вершин

	// 1. Создание итератора по ребрам.
	sc_iterator* edges_it = s->create_iterator(
		sc_constraint_new(
			CONSTR_5_f_a_a_a_f,
			graph,
			SC_A_CONST|SC_POS,
			0,
			SC_A_CONST|SC_POS,
			graph_theory::edge_
	), true);

	// 2. Вывод ребер.
	sc_for_each (edges_it) {
		sc_addr edge = edges_it->value(2);

		// Получим вершины, инцидентные ребру edge.
		sc_addr v1 = 0, v2 = 0;
		get_edge_vertexes(s, edge, v1, v2);

		// Выведем ребро вместе с инцидентными вершинами.
		std::cout << s->get_idtf(v1) << " -- " << s->get_idtf(v2) << std::endl;

		// Запомним вершины, как выведенные.
		printed_vertexes.insert(v1);
		printed_vertexes.insert(v2);
	}

	// 3. Вывод вершин, которые не имеют инцидентных ребер.
	sc_iterator *vertexes_it = s->create_iterator(
		sc_constraint_new(
			CONSTR_5_f_a_a_a_f,
			graph,
			SC_A_CONST|SC_POS,
			0,
			SC_A_CONST|SC_POS,
			graph_theory::vertex_
	), true);
	sc_for_each (vertexes_it) {
		sc_addr vertex = vertexes_it->value(2);

		// Проверим, входит ли вершина в множество printed_vertexes
		if(printed_vertexes.find(vertex) == printed_vertexes.end())
			std::cout << s->get_idtf(vertex) << '\n';
	}
}

/// Находит начало структуры маршурта @p route_struct.
sc_addr get_route_struct_begin(sc_session *s, sc_addr route_struct)
{
	// Начальной считается вершина, в которую нет входящих связок.
	// Переберем все вершины и проверим их на это свойство.
	//

	sc_iterator *it = s->create_iterator(
		sc_constraint_new(
			CONSTR_5_f_a_a_a_f,
			route_struct,
			SC_A_CONST|SC_POS,
			0,
			SC_A_CONST|SC_POS,
			graph_theory::vertex_
	), true);

	sc_for_each (it) {
		sc_addr vertex = it->value(2);

		// Ориентированный граф структуры маршрута можно рассматривать как
		// бинарное ориентированное отношение, которое связывает вершины.
		// Это позволяет нам использовать для проверки метод sc_rel::bin_ord_at_1.
		//
		if (!sc_rel::bin_ord_at_1(s, route_struct, vertex))
			return vertex;
	}

	return 0;
}

/// Возвращает структуру маршрута для связки отношения маршрут*.
/// @note Структура маршрута - это компонент с атрибутом 1_.
inline sc_addr get_route_struct(sc_session *s, sc_addr route)
{
	return sc_tup::at(s, route, N1_);
}

/// Возвращает отношение посещения для связки отношения маршрут*.
/// @note Отношение посещения - это компонент с атрибутом 3_.
inline sc_addr get_route_visit(sc_session *s, sc_addr route)
{
	return sc_tup::at(s, route, N3_);
}

/// Выводит на консоль маршрут, получив связку отношения @p route.
void print_route(sc_session *s, sc_addr route)
{
	sc_addr route_struct = get_route_struct(s, route);
	sc_addr route_visit  = get_route_visit(s, route);

	sc_addr cur_visit      = get_route_struct_begin(s, route_struct);
	sc_addr visited_vertex = sc_rel::bin_ord_at_2(s, route_visit, cur_visit);

	std::cout << s->get_idtf(visited_vertex);
	while (true) {
		cur_visit = sc_rel::bin_ord_at_2(s, route_struct, cur_visit);

		if (cur_visit) {
			visited_vertex = sc_rel::bin_ord_at_2(s, route_visit, cur_visit);
			std::cout << " -> " << s->get_idtf(visited_vertex);
		} else {
			break;
		}
	}

	std::cout << '\n';
}

/// Возвращает вершину, которая инцидентена ребру @p edge и не является вершиной @p vertex.
sc_addr get_other_vertex_incidence_edge(sc_session *s, sc_addr edge, sc_addr vertex)
{
	sc_addr v1 = 0, v2 = 0;
	get_edge_vertexes(s, edge, v1, v2);
	return vertex == v1 ? v2 : v1;
}

/// Добавляет посещение вершины @p vertex в маршрут @p route.
/// Возвращает посещение вершины (вершина в ориентированном графе структуры маршрута).
///
/// @note Все элементы генерируются в сегменте связки @p route.
sc_addr add_vertex_visit_to_route(sc_session *s, sc_addr route, sc_addr vertex)
{
	sc_addr route_struct = get_route_struct(s, route);
	sc_addr route_visit  = get_route_visit(s, route);

	sc_addr vertex_visit = sc_rel::bin_ord_at_1(s, route_visit, vertex);
	if (vertex_visit == 0) {
		vertex_visit = s->create_el(route->seg, SC_N_CONST);
		sc_tup::add(s, route->seg, route_struct, graph_theory::vertex_, vertex_visit);
		sc_rel::add_ord_tuple(s, route_visit, vertex_visit, vertex);
	}

	return vertex_visit;
}

/// @brief Добавляет посещение ребра @p edge и инцидентные ей вершины в маршрут @p route.
///
/// @note Все sc-элементы генерируются в sc-сегменте связки отношения маршрут* @p route.
///
/// @param s           сессия для работы с sc-памятью.
/// @param route       связка отношения маршрут*.
/// @param edge        ребро, посещение которого надо добавить в маршрут.
/// @param from_vertex вершина, из которой выходит при посещении.
/// @param to_vertex   вершина, в которую входим при посещении.
///
/// @return посещение ребра (дуга в ориентированном графе структуры маршрута).
///
sc_addr add_edge_visit_to_route(sc_session *s, sc_addr route, sc_addr edge, sc_addr from_vertex, sc_addr to_vertex)
{
	sc_addr route_struct = get_route_struct(s, route);
	sc_addr route_visit  = get_route_visit(s, route);

	// Добавим посещение вершины @p from_vertex в маршрут.
	sc_addr from_vertex_visit = add_vertex_visit_to_route(s, route, from_vertex);

	// Ориентированная связка, которая обозначает в структуре маршрута посещение ребра графа, на котором задан мартшрут.
	//
	sc_addr edge_visit = s->create_el(route->seg, SC_N_CONST);

	// Добавим @p edge_visit как дугу в структуру маршрута.
	//
	sc_tup::add(s, route_struct, graph_theory::arc_, edge_visit);

	// Укажем, что @p edge_visit является посещением @p edge.
	//
	sc_rel::add_ord_tuple(s, route_visit, edge_visit, edge);

	// Укажем, что дуга @p edge_visit выходит из @p from_vertex_visit.
	//
	sc_tup::add(s, edge_visit, N1_, from_vertex_visit);

	// Найдем посещение для вершины @p to_vertex.
	//
	sc_addr to_vertex_visit = sc_rel::bin_ord_at_1(s, route_visit, to_vertex);

	// Укажем, что дуга @p edge_visit входит в @p to_vertex_visit.
	//
	sc_tup::add(s, edge_visit, N2_, to_vertex_visit);

	return edge_visit;
}

/// Ищет для @p vertex любую смежную вершину из волны @p prev_wave в графе @p graph.
///
/// @param s         сессия для работы с sc-памятью.
/// @param graph     обрабатываемый неориентированный граф.
/// @param vertex    вершина, для которой ищется смежная вершина из @p prev_wave.
/// @param prev_wave предыдущая волна, из которой ищется смежная вершина.
///
/// @return ребро инцидентное @p vertex и найденной вершине.
sc_addr find_any_edge(sc_session *s, sc_addr graph, sc_addr vertex, sc_addr prev_wave)
{
	sc_addr edge = 0;

	sc_iterator *it = s->create_iterator(
		sc_constraint_new(
			CONSTR_3l2_f_a_a_a_f,
			graph,
			SC_A_CONST|SC_POS,
			SC_N_CONST,
			SC_A_CONST|SC_POS,
			vertex
	), true);

	sc_for_each (it) {
		edge = it->value(2);
		sc_addr other_vertex = get_other_vertex_incidence_edge(s, edge, vertex);

		if(sc_set::is_in(s, other_vertex, prev_wave))
			break;
	}

	return edge;
}

/// Удаляет все волны из списка и список волн, начиная с элемента списка @p waves_list_head.
void erase_waves_list(sc_session *s, sc_addr waves_list_head)
{
	sc_addr waves_list_curr = waves_list_head;
	while (waves_list_curr) {
		// Удаляем волну.
		//
		sc_addr curr_wave = sc_list::get_value(s, waves_list_curr);
		s->erase_el(curr_wave);

		// Удаляем текущий элемент списка.
		//
		sc_addr waves_list_next = sc_list::get_next(s, waves_list_curr);
		s->erase_el(waves_list_curr);

		waves_list_curr = waves_list_next;
	}
}

/// Создает следующую волну из непроверенных вершин,
/// смежных с вершинами из волны @p wave неориентированного графа @p graph.
///
/// @param s                    сессия для работы с sc-памятью.
/// @param seg                  сегмент, в котором происходит генерация новой волны.
/// @param graph                обрабатываемый неориентированный граф.
/// @param wave                 текущая волна.
/// @param not_checked_vertexes множество непроверенных вершин.
///
/// @return созданная новая волна.
sc_addr create_wave(sc_session *s, sc_segment *seg, sc_addr graph, sc_addr wave, sc_addr not_checked_vertexes)
{
	// Создадим узел новой волны.
	sc_addr new_wave = s->create_el(seg, SC_N_CONST);

	// 1. Перебор всех вершин из волны wave.
	sc_iterator *it_vertex = s->create_iterator(
		sc_constraint_new(
			CONSTR_3_f_a_a,
			wave,
			SC_A_CONST|SC_POS,
			0
		), true);
	sc_for_each (it_vertex) {
		sc_addr vertex = it_vertex->value(2);

		// 2. Перебор всех ребер, которые инцидентны vertex.
		sc_iterator *it_edge = s->create_iterator(
			sc_constraint_new(
				CONSTR_3l2_f_a_a_a_f,
				graph,
				SC_A_CONST|SC_POS,
				0,
				SC_A_CONST|SC_POS,
				vertex
			), true);
		sc_for_each (it_edge) {
			sc_addr edge = it_edge->value(2); // ребро, инцидентное  vertex
			sc_addr other_vertex = get_other_vertex_incidence_edge(
				s, edge, vertex); // вершина, смежная vertex и инцидентная ребру

			// Исключим вершину other_vertex из множества непроверенных вершин и
			// включим в создаваемую волну.
			if (sc_set::exclude_from(s, other_vertex, not_checked_vertexes))
				sc_set::include_in(s, other_vertex, new_wave);
		}
	}

	return new_wave;
}

/// Находит один из минимальных путей в графе @p graph
/// от вершины @p beg_vertex до вершины @p end_vertex.
///
/// @param s          сессия для работы с sc-памятью.
/// @param seg        сегмент, в котором происходит работа алгоритма.
/// @param graph      неориентированный граф, в котором будет находится минимальный путь.
/// @param beg_vertex начальная вершина пути.
/// @param end_vertex конечная вершина пути.
///
/// @return связка отношения "простая цепь*" или 0, если минимальный путь не найден.
sc_addr find_min_path(sc_session *s, sc_segment *seg, sc_addr graph, sc_addr beg_vertex, sc_addr end_vertex)
{
	// 1. Добавим все вершины графа (кроме начальной вершины пути)
	// в множество непроверенных вершин.

	// множество непроверенных вершин
	sc_addr not_checked_vertexes = s->create_el(seg, SC_N_CONST);

	// Перебор всех вершин.
	sc_iterator *it = s->create_iterator(
		sc_constraint_new(
			CONSTR_5_f_a_a_a_f,
			graph,
			SC_A_CONST|SC_POS,
			0,
			SC_A_CONST|SC_POS,
			graph_theory::vertex_
		), true);
	sc_for_each (it) {
		sc_addr vertex = it->value(2);

		// Не добавляем вершину начала пути в множество непросмотренных вершин.
		if (vertex != beg_vertex)
			sc_set::include_in(s, it->value(2), not_checked_vertexes);
	}

	// 2. Создадим начальную волну и добавим в нее начальную вершину пути.
	// Включим в список волн.
	sc_addr new_wave = s->create_el(seg, SC_N_CONST);
	sc_set::include_in(s, beg_vertex, new_wave);

	// Создадим начало списка волн.
	sc_addr waves_list_head = sc_list::create(s, seg, new_wave);
	sc_addr waves_list_tail = waves_list_head;

	// 3. Сформируем список волн.
	do {
		// Создадим новую волну на основе предыдущей
		new_wave = create_wave(s, seg, graph, new_wave, not_checked_vertexes);

		// Если новая волна пуста, то значит между вершинами не существует пути.
		if (sc_set::is_empty(s, new_wave)) {
			// Очищаем память и завершаем алгоритм.
			erase_waves_list(s, waves_list_head);
			s->erase_el(new_wave);
			s->erase_el(not_checked_vertexes);
			return 0;
		}

		// Добавим новую волну в конец списка.
		sc_addr waves_list_curr = sc_list::create(s, seg, new_wave);
		sc_list::set_next(s, waves_list_tail, waves_list_curr);

		waves_list_tail = waves_list_curr;

		// Если в новой волне есть конечная вершина, то перейдем в начало цикла.
	} while (!sc_set::is_in(s, end_vertex, new_wave));

	// Подчистим память.
	s->erase_el(not_checked_vertexes);

	// 4. Создадим связку отношения простая цепь*.
	sc_addr route        = s->create_el(seg, SC_N_CONST); // связка отношения "простая цепь*"
	sc_set::include_in(s, route, graph_theory::simple_trail);

	// 5. Создадим компоненты связки route.
	sc_addr route_struct = s->create_el(seg, SC_N_CONST); // ориентированный граф структуры маршрута
	sc_set::include_in(s, route_struct, graph_theory::directed_graph);
	sc_addr route_visit  = s->create_el(seg, SC_N_CONST); // бинарное отношение посещения

	// 6. Добавим все компоненты в связку route.
	sc_tup::add(s, route, N1_, route_struct);
	sc_tup::add(s, route, N2_, graph);
	sc_tup::add(s, route, N3_, route_visit);

	// 7. Добавим в простую цепь посещение начальной и конечной вершин.
	sc_addr beg_vertex_visit = add_vertex_visit_to_route(s, route, beg_vertex);
	sc_addr end_vertex_visit = add_vertex_visit_to_route(s, route, end_vertex);

	// Пройдем в обратном направлении по списку волн
	// и сформируем структуру маршрута.
	//

	sc_addr curr_vertex = end_vertex;

	// Строим из списка волн маршрут, проходя по этому списку в обратном направлении.
	sc_list::reverse_iterator list_it(s, waves_list_tail), list_end;
	for (++list_it; list_it != list_end; ++list_it) {
		sc_addr curr_wave = *list_it;

		sc_addr edge = find_any_edge(s, graph, curr_vertex, curr_wave);

		// Получаем предыдущую вершину в пути.
		//
		sc_addr prev_vertex = get_other_vertex_incidence_edge(s, edge, curr_vertex);

		// Добавляем посещение ребра @p edge в путь.
		//
		add_edge_visit_to_route(s, route, edge, prev_vertex, curr_vertex);
		curr_vertex = prev_vertex;
	}

	// Подчистим память...
	//
	erase_waves_list(s, waves_list_head);

	return route;
}

/// Подготавливает запуск и запускает тестовый пример для алгоритма поиска минимального пути.
///
/// @param s          sc-сессия, при помощи которой будет производиться работа с sc-памятью.
/// @param number     порядковый номер тестового примера.
/// @param graph_str  строка, задающая тестовый граф для функции #gen_undirected_graph.
/// @param beg_name   имя начальной вершина в графе @p graph_str для поиска минимального пути.
/// @param end_name   имя конечной вершина в графе @p graph_str для поиска минимального пути.
///
/// @see gen_undirected_graph
/// @see find_min_path
void run_testcase(sc_session *s, int number, const sc_string &graph_str, const sc_string &beg_name, const sc_string &end_name)
{
	std::cout << "[Testcase " << number << "]\n";

	// Для работы создаем временный сегмент /tmp/wave_find_path.
	//
	sc_segment* tmp_seg = create_unique_segment(s, "/tmp/wave_find_path");

	// Генерируем текстовый граф в sc-памяти.
	//
	sc_addr graph = gen_undirected_graph(s, tmp_seg, graph_str);

	// Распечатаем граф на консоль
	//
	std::cout << "Graph string: \n" << graph_str << '\n';

	std::cout << "Graph from memory: " << '\n';
	print_graph(s, graph);

	// Найдем начальную вершину для тестового поиска минимального пути.
	//
	sc_addr beg = s->find_by_idtf(beg_name, tmp_seg);
	assert(beg);

	// Найдем конечную вершину для тестового поиска миниального пути.
	//
	sc_addr end = s->find_by_idtf(end_name, tmp_seg);
	assert(end);

	std::cout << "Find minimal path from '" << beg_name << "' to '" << end_name << "'\n";

	// Найдем минимальный пути между начальной и конечной вершинами.
	//
	sc_addr result = find_min_path(s, tmp_seg, graph, beg, end);

	// Распечатаем найденный путь на консоль
	std::cout << "Path";

	if (result) {
		std::cout << ":\n";
		print_route(s, result);
	} else {
		std::cout << " isn't exist.\n";
	}

	// Удалим временный sc-сегмент.
	//
	s->unlink(tmp_seg->get_full_uri());

	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	// 1. Инициализация libsc.
	sc_session *system = libsc_init();

	// Теперь мы получили пустую sc-память.

	// 2. Создание sc-сегмента \verb|/proc/keynode| и
	// системных ключевых узлов в нем (например, \verb|1_|, \verb|2_| и т.д.)
	// Эту функцию нужно вызывать обязательно.
	pm_keynodes_init(system);

	// 3. Инициализируем ключевые узлы базы знаний по теории графов.
	graph_theory::init(system);

	// 4. Создадим пользовательскую сессию для работы алгоритма.
	sc_session *session = libsc_login();

	// 5. Откроем сегменты системных ключевых узлов и ключевых узлов
	// базы знаний по теории графов в рамках пользовательской сессии.
	session->open_segment("/proc/keynode");
	session->open_segment(graph_theory::segment_uri);

    // 6. Запуск тестов алгоритма.
    run_testcase(1, "graph1.txt", "A", "C");
    run_testcase(2, "graph2.txt", "A", "F");
    run_testcase(3, "graph3.txt", "A", "K");
    run_testcase(4, "graph4.txt", "V5", "V11");
    run_testcase(5, "graph5.txt", "V1", "V9");

    // 7. Закроем пользовательскую сессию.
    session->close();

    // 8. Деинициализируем libsc.
    libsc_deinit();


	return 0;
}
