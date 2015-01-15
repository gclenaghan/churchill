#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "point_search.h"

#include <iostream>

class TreeNode
{
public:
	TreeNode(Point point);
	TreeNode();
	~TreeNode();
	Point p;
	TreeNode *ne;
	TreeNode *se;
	TreeNode *sw;
	TreeNode *nw;

	std::vector<Point *> *search(const Rect rect, const int32_t count);
	void insert(Point point);
};

struct SearchContext
{
	SearchContext(const Point* points_begin, const Point* points_end);
	int32_t search(const Rect rect, const int32_t count, Point* out_points);
	std::vector<Point> *points;

	TreeNode quadtree;
};

inline bool is_inside(const Rect &rect, const Point &point)
{
	if( rect.lx <= point.x && rect.hx >= point.x &&
		rect.ly <= point.y && rect.hy >= point.y )
		return true;
	return false;
}

inline bool sortpoint(const Point a, const Point b)
{
	return a.rank < b.rank;
}

//SearchContext definitions-------------------------------------------------------------------------------------
SearchContext::SearchContext(const Point* points_begin, const Point* points_end)
{
	points = new std::vector<Point>(points_begin, points_end);
	std::sort(points->begin(), points->end(), sortpoint);

	for (auto it : *points)
	{
		quadtree.insert(it);
	}

	delete points;
}

int32_t SearchContext::search(const Rect rect, const int32_t count, Point* out_points)
{
	std::vector<Point *> *result = quadtree.search(rect, count);
	if (result)
	{
		int32_t s = result->size();

		for (auto it : *result)
		{
			*out_points = *it;
			out_points++;
		}
		return s;
	}
	return 0;
}

//TreeNode definitions-------------------------------------------------------------------------------------------
TreeNode::TreeNode(Point point) : ne(NULL) , se(NULL) , sw(NULL) , nw(NULL)
{
	p = point;
}
TreeNode::TreeNode() : ne(NULL) , se(NULL) , sw(NULL) , nw(NULL)
{
	p.rank = 0;
}
TreeNode::~TreeNode()
{
	delete ne;
	delete se;
	delete sw;
	delete nw;
}

void TreeNode::insert(Point point)
{
	if (p.rank == 0)
	{
		p = point;
		return;
	} else if (point.x > p.x)
	{
		if (point.y > p.y)
		{
			if (ne)
			{
				ne->insert(point);
			} else {
				ne = new TreeNode(point);
			}
		} else {
			if (se)
			{
				se->insert(point);
			} else {
				se = new TreeNode(point);
			}
		}
	} else {
		if (point.y > p.y)
		{
			if (nw)
			{
				nw->insert(point);
			} else {
				nw = new TreeNode(point);
			}
		} else {
			if (sw)
			{
				sw->insert(point);
			} else {
				sw = new TreeNode(point);
			}
		}
	}
}

std::vector<Point *> *twonodes(TreeNode *node1, TreeNode *node2, const Rect rect, const int32_t count)
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
		std::vector<Point *> *list1 = node1->search(rect, count);
		if (list1)
		{
			std::vector<Point *> *list2 = node2->search(rect, count);
			if (list2)
			{
				std::vector<Point *> *merged = new std::vector<Point *>;
				merged->reserve(count);
				for (int c = 0, i = 0, j = 0; c < count; c++)
				{
					if ((*list1)[i] == *list1->end())
					{ //one of the lists is exhausted, copy the rest from the other
						if ((*list2)[j] == *list2->end())
						{
							break;
						}
						merged->push_back((*list2)[j]);
						j++;
					} else if ((*list2)[j] == *list2->end())
					{
						merged->push_back((*list1)[i]);
						i++;
					} else { //both lists still have elements
						if ((*list1)[i]->rank < (*list2)[j]->rank)
						{
							merged->push_back((*list1)[i]);
							i++;
						} else {
							merged->push_back((*list2)[j]);
							j++;
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


std::vector<Point *> *TreeNode::search(const Rect rect, const int32_t count)
{ //searches down the tree for points in given rectangle. Returns sorted list of points in rect, at least as much as count unless out of points
	if (p.rank == 0)
	{
		return NULL;
	}
	char bitmask = (((rect.lx <= p.x) ? 1 : 0) | ((rect.hx >= p.x) ? 2 : 0) | ((rect.ly <= p.y) ? 4 : 0) | ((rect.hy >= p.y) ? 8 : 0));

	switch (bitmask)
	{
	case 5:
		return (sw ? sw->search(rect, count) : NULL);
	case 6:
		return (se ? se->search(rect, count) : NULL);
	case 7:
		return twonodes(sw, se, rect, count);
	case 9:
		return (nw ? nw->search(rect, count) : NULL);
	case 10:
		return (ne ? ne->search(rect, count) : NULL);
	case 11:
		return twonodes(nw, ne, rect, count);
	case 13:
		return twonodes(nw, sw, rect, count);
	case 14:
		return twonodes(ne, se, rect, count);
	case 15:
	{
		std::vector<Point *> *merged = new std::vector<Point *>, *list1, *list2;
		merged->reserve(count);
		merged->push_back(&p);

		if (count < 2)
		{
			return merged;
		}

		list1 = twonodes(ne, nw, rect, count - 1);
		list2 = twonodes(se, sw, rect, count - 1);

		if (list1)
		{
			if (list2)
			{
				for (int c = 0, i = 0, j = 0; c < count; c++)
				{
					if ((*list1)[i] == *list1->end())
					{ //one of the lists is exhausted, copy the rest from the other
						if ((*list2)[j] == *list2->end())
						{
							break;
						}
						merged->push_back((*list2)[j]);
						j++;
					} else if ((*list2)[j] == *list2->end())
					{
						merged->push_back((*list1)[i]);
						i++;
					} else { //both lists still have elements
						if ((*list1)[i]->rank < (*list2)[j]->rank)
						{
							merged->push_back((*list1)[i]);
							i++;
						} else {
							merged->push_back((*list2)[j]);
							j++;
						}
					}
				}
			} else {
				merged->insert(merged->end(), list1->begin(), list1->end());
			}
		} else {
			if (list2)
			{
				merged->insert(merged->end(), list2->begin(), list2->end());
			} else {
				return merged;
			}
		}

		delete list1;
		delete list2;
		return merged;
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
