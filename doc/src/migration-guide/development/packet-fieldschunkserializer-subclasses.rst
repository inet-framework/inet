Migrating ``FieldsChunkSerializer`` Subclasses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`FieldsChunkSerializer` has two protected hooks for each direction. The old
pair does not see the requested chunk type:

.. code-block:: c++

   void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
   const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

The new pair does:

.. code-block:: c++

   void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
   const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;

The old pair is deprecated, but it still works and it stays overridable. The new
hooks call the old hooks by default, so an existing serializer needs no source
change.

Override the new pair in new code. The ``typeInfo`` parameter names the concrete
chunk type that was requested from :cpp:`ChunkSerializerRegistry`. A serializer
that is registered for several chunk types must inspect ``typeInfo`` to build the
exact requested type; the old hook cannot do this and always builds one type. A
serializer that produces a single chunk type may leave the parameter unnamed.

One call between field serializers must use the new name as well, because the
default body of ``deserializeFields()`` is what forwards to the old hook:

.. code-block:: c++

   return OtherSerializer().deserializeFields(stream, typeid(OtherChunk));
