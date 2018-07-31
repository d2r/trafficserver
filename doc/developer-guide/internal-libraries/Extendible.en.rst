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

.. |FieldId| replace:: :class:`FieldId`
.. |FieldId_C| replace:: :class:`FieldId_C`
.. |FieldSchema| replace:: :class:`FieldSchema`
.. |Schema| replace:: :class:`Schema`
.. |Extendible| replace:: :class:`Extendible`

Extendible
**********

Synopsis
++++++++

.. code-block:: cpp

   #include "ts/Extendible.h"

|Extendible| allows Plugins to append additional storage to Core data structures.
Use Case:

TSCore

  Defines class Host as Extendible

TSPlugin HealthStatus

  Appends Field "down reason code" to Host
  Now the plugin does not need to create completely new lookup table and all implementation that comes with it.


Description
+++++++++++

A data class that inherits from Extendible, uses a CRTP (Curiously Recurring Template Pattern) so that its static Schema instance is unique among other |Extendible| types.

.. code-block:: cpp

   class ExtendibleExample : Extendible<ExtendibleExample> {
      int real_member = 0;
   }

Structures
----------
* |FieldId|:code:`<AccessType_, FieldType>` - the handle used to access a field. These are templated to prevent human error, and branching logic.
* |FieldId_C| - the handle used to access a field through C API. Human error not allowed by convention.
* |FieldSchema| - stores attributes, constructor and destructor of a field.
* |Schema| - manages fields and memory layout of an |Extendible| type.
* |Extendible|:code:`<DerivedType>` - allocates block of memory, uses |FieldId| or schema to access slices of memory.


During system init, code and plugins can add fields to the Extendible's schema. This will update the `Memory Layout`_ of the schema, and the memory offsets of all fields. The schema does not know the FieldType, but it stores the byte size and creates lambdas of the type's constructor and destructor. And to avoid corruption, the code asserts that no instances are in use when adding fields.

.. code-block:: cpp

   ExtendibleExample:FieldId<ATOMIC,int> fld_my_int;

   void PluginInit() {
     fld_my_int = ExtendibleExample::schema.addField("my_plugin_int");
   }


When an |Extendible| derived class is instantiated, new() will allocate a block of memory for the derived class and all added fields. There is zero memory overhead per instance, unless using :ref:ACIDPTR field access type.

.. code-block:: cpp

   ExtendibleExample* alloc_example() {
     return new ExtendibleExample();
   }

Memory Layout
-------------
One block of memory is allocated per Extendible, which include all member variables and appended fields.
Within the block, memory is arranged in the following order:

#. Derived members (+padding align next field)
#. Fields (largest to smallest)
#. Packed Bits

Strongly Typed Fields
---------------------
:code:`FieldId<AccessType_, FieldType>`

|FieldId| is a templated Field reference. One benefit is that all type casting is internal to the |Extendible|,
and simplifies the code using it. Also this provides compile errors for common misuses and type mismatches.

.. code-block:: cpp

   // Core code
   class Food : Extendible<Food> {}
   class Car : Extendible<Car> {}

   // Example Plugin
   Food::FieldId<STATIC,float> fld_food_weight;
   Food::FieldId<STATIC,time_t> fld_expr_date;
   Car::FieldId<STATIC,float> fld_max_speed;
   Car::FieldId<STATIC,float> fld_car_weight;

   PluginInit() {
      Food.schema.addField(fld_food_weight, "weight");
      Food.schema.addField(fld_expr_date,"expire date");
      Car.schema.addField(fld_max_speed,"max_speed");
      Car.schema.addField(fld_car_weight,"weight"); // 'weight' is unique within 'Car'
   }

   PluginFunc() {
      Food banana;
      Car camry;

      // Common user errors

      float expire_date = banana.get(fld_expr_date);
      //^^^                                              Compile error: cannot convert time_t to float
      float speed = banana.get(fld_max_speed);
      //                       ^^^^^^^^^^^^^             Compile error: fld_max_speed is not of type Extendible<Food>::FieldId
      float weight = camry.get(fld_food_weight);
      //                       ^^^^^^^^^^^^^^^           Compile error: fld_food_weight is not of type Extendible<Car>::FieldId
   }

Field Access Types
------------------
.. _AccessType:

============   =======================================   ============================================================   ========================================   ================================================
Enums          Allocates                                 API                                                            Pros                                       Cons
============   =======================================   ============================================================   ========================================   ================================================
ATOMIC         std::atomic<FieldType>                    :code:`get(FieldId)`                                           Leverages std::atomic<> API. No Locking.   Only works on small data types.
BIT            1 bit from packed bits                    :code:`get(FieldId), readBit(FieldId), writeBit(FieldId)`      Memory efficient.                          Cannot return reference.
:ref:ACIDPTR   shared_ptr<FieldType> = new FieldType()   :code:`get(FieldId), writeAcidPtr(FieldId)`                    Avoid skew in non-atomic structures.       Non-contiguous memory allocations. Uses locking.
STATIC         FieldType                                 :code:`get(FieldId), init(FieldId)`                            Const reference. Fast. Type safe.          No concurrency protection.
DIRECT         FieldType                                 :code:`get(FieldId)`                                           Direct reference. Fast. Type safe.         No concurrency protection.
C_API          a number of bytes                         :code:`get(FieldId_C)`                                         Can use in C.                              No concurrency protection. Not type safe.
============   =======================================   ============================================================   ========================================   ================================================

:code:`operator[](FieldId)` has been overridden to call :code:`get(FieldId)` for all access types.

Unfortunately our data is not "one type fits all". I expect that most config values will be stored as STATIC, most states values will be ATOMIC or BIT, while vectored results will be :ref:ACIDPTR.
