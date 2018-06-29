.. Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
   distributed with this work for additional information
   regarding copyright ownership.  The ASF licenses this file
   to you under the Apache License, Version 2.0 (the
   "License"); you may not use this file except in compliance
   with the License.  You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.

.. include:: ../../common.defs
.. highlight:: cpp
.. default-domain:: cpp

.. _MemArena:

MemArena
*************

:class:`MemArena` provides a memory arena or pool for allocating memory. Internally
:class:`MemArena` allocates memory in large blocks and allocates pieces of those blocks when memory
is requested. Upon destruction all of the memory blocks are de-allocated. This is useful when the
goal is any (or all) of trying to

*  amortize allocation costs for many small allocations.
*  create better memory locality for containers.
*  de-allocate memory in bulk.

Description
+++++++++++

When a :class:`MemArena` instance is constructed it can be constructed with either no allocated
memory or an initial memory block of a specific size. :func:`MemArena::reserve` can be used to
specify the size of the next internally allocated block if a specific allocation size is needed.
This allows allocation to be delayed until memory is actually needed (via a call to
:func:`MemArena::alloc`).

In normal use memory is allocated from :class:`MemArena` using :func:`MemArena::alloc` to get chunks
of memory, or :func:`MemArena::make` to get constructed class instances. :func:`MemArena::make`
takes an arbitrary set of arguments which it attempts to pass to a constructor for the type
:code:`T` after allocating memory (:code:`sizeof(T)` bytes) for the object. If there isn't enough
free memory, a new internal block is allocated. Each time a new internal block is allocated, the
block than the previously allocated one. In general this suffices but in certain cases, if it is
necessary adjust the internal allocation size, he size can be controlled using
:func:`MemArena::reserve`. Other than a freeze / thaw cycle, there is no mechanism to release memory
except for the destruction of the :class:`MemArena`. In such use cases either wasted memory must be
small enough or temporary enough to not be an issue, or there must be a provision for some sort of
garbage collection.

Generally :class:`MemArena` is not as useful for classes that allocate their own internal memory
(such as :code:`std::string` or :code:`std::vector`), which includes most container classes. One
container class that can be easily used is :class:`IntrusiveDList` because the links are in the
instance and therefore also in the arena.

After a series of allocations, the allocated memory can be in different allocation blocks, along
with possibly no longer in use memory. In such a situation it can be useful to coalesce (or garbage
collect) all of the in use memory in the arena into a single bulk allocation block. This improves
memory efficiency and memory locality. The method :func:`MemArena::freeze` is provided as a base
mechanism for coalescence. Calling :func:`MemArena::freeze` with an argument of :code:`true` locks
down all current blocks so no more memory will be allocated from them. Any further allocations will
be in new internal memory blocks. The size of the next internal block is set to be at least as large
as the sum of the currently allocated memory, adjusting as needed. Calling :func:`MemArena::freeze`
with an argument of :code:`false` will de-allocate all of the frozen internal memory blocks. This is
convenient because coalescence can done by

#. Freezing the arena.
#. Copying all objects back in to the arena.
#. Unfreezing the arena.

Because the default next block size is large enough, all of the copied objects will be put in the
same new internal block. If this for some reason this sizing isn't correct :func:`MemArena::reserve`
can be called to specify a different value (if, for instance, there is a lot of unused memory of
known size). Generally this is most useful for process static data that is initialized on process
start. After the process start initilization, the data can be coalesced for better performance after
all modifications have been done. Alternatively, a container that allocates and de-allocates same
sized objects (such as a :code:`std::map`) can use a free list to re-use objects before going to the
:class:`MemArena` for more memory and thereby avoiding collecting unused memory in the arena.

Objects created in the arena must not have :code:`delete` called on them as this will corrupt
memory, usually leading to an immediate crash. The memory for the instance will be released when the
arena is destroyed. The destructor can be called if needed but in general if a destructor is needed
it is probably not a class that should be constructed in the arena. Looking at
:class:`IntrusiveDList` again for an example, if this is used to link objects in the arena, there is
no need for a destructor to clean up the links - all of the objects will be de-allocated when the
arena is destroyed. Whether this kind of situation can be arranged with reasonable effort is a good
heuristic on whether :class:`MemArena` is an appropriate choice.

While :class:`MemArena` will normally allocate memory from an internal chunk, if the allocation
request is large (more than a memory page) and there is not enough space in the current internal
block, a block just for that allocation will be created. This is useful if the purpose of
:class:`MemArena` is to track blocks of memory more than reduce the number of system level
allocations.

Reference
+++++++++

.. class:: MemArena

   .. function:: MemArena()

      Construct an empty arena.

   .. function:: MemArena(size_t n)

      Construct an arena with an initial internal block with at least :arg:`n` bytes of free space.

   .. function:: MemSpan alloc(size_t n)

      Allocate memory of size :arg:`n` bytes in the arena.

   .. function:: template < typename T, typename ... Args > T * make(Args&& ... args)

      Create an instance of :arg:`T`. :code:`sizeof(T)` bytes of memory are allocated from the arena
      and the constructor invoked. This method takes any set of arguments, which are passed to
      the constructor. A pointer to the newly constructed instance of :arg:`T` is returned. Note if
      the instance allocates other memory that memory will not be in the arena. Example constructing
      a :code:`std::string_view` ::

         std::string_view * sv = arena.make<std::string_view>(pointer, n);

   .. function:: MemArena& freeze(bool flag)

      If :arg:`flag` is :code:`true`, stop allocating from existing internal memory blocks. These
      blocks are now "frozen". Adjust the size of the next allocation block, if necessary, to be
      sufficiently large to hold all of the contents of the frozen memory will be created.

      If :arg:`flag` is :code:`false`, dellocate all frozen internal memory blocks.

   .. function:: MemArena& reserve(size_t n)

      Mark the next internal block to have at least :arg:`n` bytes of available space.


Internals
+++++++++

Allocated memory is tracked by two linked lists, one for current memory and the other for frozen
memory. The latter is used only while the arena is frozen. Because a shared pointer is used for the
link, the list can be de-allocated by clearing the head pointer in :class:`MemArena`. Because this
pattern is similar to that used by the :code:`IOBuffer` data blocks, those were considered for use
as the internal memory allcation blocks. However, that would have required some non-trivial tweaks
and, with the move away from internal allocation pools to memory support from libraries like
"jemalloc", unlikely to provide any benefit.
