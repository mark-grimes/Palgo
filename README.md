**Palgo** (parallel algorithms)

A header library of templated algorithms aimed at parallel running. First entry is intended to
be a [k-d tree](https://en.wikipedia.org/wiki/K-d_tree) implementation that can run using technologies
like [tbb](https://www.threadingbuildingblocks.org/) or [SYCL](https://www.khronos.org/sycl]).

Note that this library is very much in alpha at the moment and very much in flux. Very little of it is
even parallel yet. I would suggest coming back at a later time.

`fixed_kdtree` is a fully working implementation of a kd-tree that will sort itself and allow nearest
neighbour searches. It's called "fixed" because the data cannot be changed once the tree has been built;
having a mutable kd-tree is not currently a priority. The way a tree is partitioned is set by passing a
std::vector of functions to the constructor. The test coverage is not complete yet so there may be some
edge cases that give incorrect answers.
 
The current priority is to get a working version that will run on GPU/accelerators using [ComputeCpp]
(https://www.codeplay.com/products/computecpp). The challenge is that SYCL does not allow function
pointers or recursion. To get around this, the current (not yet working) approach is to use template
meta programming to specify the partitioning functions in the template parameters of `static_kdtree`,
so that all partitioning information is available at compile time (hence "static").
