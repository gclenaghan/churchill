#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>
#include <algorithm>
#include "point_search.h"

#include <iostream>

class TreeNode
{
public:
	TreeNode(Point point, const bool sn);
	TreeNode();
	~TreeNode();
	Point p;
	TreeNode *ne;
	TreeNode *sw;
	const bool ns;

	std::deque<Point *> *search(const Rect rect, const int32_t count);
	void insert(Point point);
};

struct SearchContext
{
	SearchContext(const Point* points_begin, const Point* points_end);
	int32_t search(const Rect rect, const int32_t count, Point* out_points);

	TreeNode kdtree;
};

inline bool sortpoint(const Point a, const Point b)
{
	return a.rank < b.rank;
}

//SearchContext definitions-------------------------------------------------------------------------------------
SearchContext::SearchContext(const Point* points_begin, const Point* points_end)
{
	std::deque<Point> points(points_begin, points_end);
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		if (it->x > 2000 || it->x < -2000 || it->y > 2000 || it->y < -2000)
		{
			points.erase(it);
		}
	}
	std::sort(points.begin(), points.end(), sortpoint);

	for (auto it : points)
	{
		kdtree.insert(it);
	}
}

int32_t SearchContext::search(const Rect rect, const int32_t count, Point* out_points)
{
	std::deque<Point *> *result = kdtree.search(rect, count);
	if (result)
	{
		for (auto it : *result)
		{
			*out_points = *it;
			out_points++;
		}
		return result->size();
	}
	return 0;
}

//TreeNode definitions-------------------------------------------------------------------------------------------
TreeNode::TreeNode(Point point, const bool sn) : ne(NULL) , sw(NULL) , ns(sn)
{
	p = point;
}
TreeNode::TreeNode() : ne(NULL) , sw(NULL) , ns(true)
{
	p.rank = -1;
}
TreeNode::~TreeNode()
{
	delete ne;
	delete sw;
}

void TreeNode::insert(Point point)
{
	if (p.rank == -1)
	{
		p = point;
		return;
	}

	if (ns)
	{
		if (p.y > point.y)
		{
			if (sw)
			{
				sw->insert(point);
			} else {
				sw = new TreeNode(point, false);
			}
		} else {
			if (ne)
			{
				ne->insert(point);
			} else {
				ne = new TreeNode(point, false);
			}
		}
	} else {
		if (p.x > point.x)
		{
			if (sw)
			{
				sw->insert(point);
			} else {
				sw = new TreeNode(point, true);
			}
		} else {
			if (ne)
			{
				ne->insert(point);
			} else {
				ne = new TreeNode(point, true);
			}
		}
	}
}

std::deque<Point *> *twonodes(TreeNode *node1, TreeNode *node2, const Rect rect, const int32_t count)
{ //Returns sorted list of at most count points within rect from the two nodes
	char bitmask = ((node1 ? 1 : 0) | (node2 ? 2 : 0));

	switch (bitmask)
	{
	case 0:
		return NULL;
	case 1:
		return node1->search(rect, count);
	case 2:
		return node2->search(rect, count);
	case 3:
	{
		std::deque<Point *> *list1 = node1->search(rect, count);
		if (list1)
		{
			std::deque<Point *> *list2 = node2->search(rect, count);
			if (list2)
			{
				std::deque<Point *> *merged = new std::deque<Point *>;
				for (int c = 0; c < count; c++)
				{
					if (list1->empty())
					{ //one of the lists is exhausted, copy the rest from the other
						if (list2->empty())
						{
							break;
						}
						merged->push_back(list2->front());
						list2->pop_front();
					} else if (list2->empty())
					{
						merged->push_back(list1->front());
						list1->pop_front();
					} else { //both lists still have elements
						if (list1->front()->rank < list2->front()->rank)
						{
							merged->push_back(list1->front());
							list1->pop_front();
						} else {
							merged->push_back(list2->front());
							list2->pop_front();
						}
					}
				}
				delete list1;
				delete list2;
				return merged;
			} else {
				return list1;
			}
		} else {
			return node2->search(rect, count);
		}
	}
	}
	return NULL; //never happens
}


std::deque<Point *> *TreeNode::search(const Rect rect, const int32_t count)
{ //searches down the tree for points in given rectangle. Returns sorted list of points in rect, at least as much as count unless out of points
	if (p.rank == -1)
	{
		return NULL;
	}
	char bitmask = (((rect.lx <= p.x) ? 1 : 0) | ((rect.hx >= p.x) ? 2 : 0) | ((rect.ly <= p.y) ? 4 : 0) | ((rect.hy >= p.y) ? 8 : 0) | (ns ? 16 : 0));

	switch (bitmask)
	{
	case 5:
	case 9:
	case 13:
	case 21:
	case 22:
	case 23:
		return (sw ? sw->search(rect, count) : NULL);
	case 6:
	case 10:
	case 14:
	case 25:
	case 26:
	case 27:
		return (ne ? ne->search(rect, count) : NULL);
	case 7:
	case 11:
	case 29:
	case 30:
		return twonodes(sw, ne, rect, count);
	case 15:
	case 31:
	{
		std::deque<Point *> *list = NULL;

		if (count < 2)
		{
			list = new std::deque<Point *>;
			list->push_back(&p);
			return  list;
		}

		list = twonodes(ne, sw, rect, count - 1);

		if (list)
		{
			list->push_front(&p);
		} else {
			list = new std::deque<Point *>;
			list->push_back(&p);
		}
		return list;
	}
	}

	return NULL; //never happens
}

//External calls--------------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) SearchContext* __stdcall create(const Point* points_begin, const Point* points_end)
{
	return new SearchContext(points_begin, points_end);
}

extern "C" __declspec(dllexport) SearchContext* __stdcall destroy(SearchContext* sc)
{
	delete sc;
	return 0;
}

extern "C" __declspec(dllexport) int32_t __stdcall search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	return sc->search(rect, count, out_points);
}
