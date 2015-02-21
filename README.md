# Ranked Point Search Algorithm for Churchill Navigation's challenge

## Challenge details
This was my submission for Churchill Navigation's challenge, which came in 38th place of 54 entries. Oh well.

See details of the challenge here [here](http://churchillnavigation.com/challenge/) as well as the github repository with all the entries [here](https://github.com/churchillnavigation/challenge1).

A summary of the problem is
* you are given 10 million points in a plane, each with a rank from 1 to 10 million,
* you can place these points in whatever data structure you want up to 512MB, the time here does not matter,
* then you are given random rectangles (sides parallel to coordinate axes) and must return the top 20 ranked points in the rectangle.

The algorithm is ranked on the time taken in the last stage.

## Submission details
The data structure used is a kd-tree, which with 2 64-bit pointers per point takes the large majority of the allocated memory.

The points are sorted and then loaded in the kd-tree, so each point is ranked strictly higher than every point below it.

The search algorithm is a breadth first exploration of the tree: at the start, the root is loaded into a priority queue, then the queue is looped over. Each loop, the top-ranked node is popped off and we check whether the point is in the rectangle (in which case it is added to the list since we have already searched all possible higher ranked points) and we check which or both of the child nodes should be added to the priority queue. This continues until finding the desired number of points or exhausting the tree.

### Potential improvements
After submitting this, I changed the tree to be a point quadtree, so each node has four children. This is around a 50% improvement in time, but occupies too much memory. A hybrid solution of three children fit in the requirements and kept most of the time advantage, but I never submitted it.

The challenge entries were run on a quadcore processor, which this does not take advantage of. One avenue may be to offload the maintenence of the priority queue to another thread. This queue is used in a manner that it pops off the top ranked and then always adds something (if anything) of a lower rank, so there is structure which is not taken advantage of in just using the STL implementation. I did try using other implementations of a priority queue (from libboost) which lead to much slower searches.

The most glaring issue is there is no low level optimization, e.g. no SSE calls or worrying about aligning memory properly. This was not my focus in coding this.
