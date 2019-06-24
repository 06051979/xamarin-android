#ifndef INC_MONODROID_INMEMORY_ASSEMBLIES_H
#define INC_MONODROID_INMEMORY_ASSEMBLIES_H

#include "dylib-mono.h"
#include "jni-wrappers.h"
#include <string.h>

#define DEFAULT_CAPACITY 8

namespace xamarin { namespace android { namespace internal {
	class InMemoryAssemblies
	{
		private:
			struct InMemoryAssemblyEntry
			{
				int domain_id;
				int assemblies_count;
				char **names;
				char **assemblies_bytes;
				int *assemblies_bytes_len;
			};

		public:
			InMemoryAssemblies ()
			{
				capacity = DEFAULT_CAPACITY;
				entries = new InMemoryAssemblyEntry*[DEFAULT_CAPACITY];
			}

			bool has_assemblies () const { return length > 0; }
			MonoAssembly* load_assembly_from_memory (MonoDomain *domain, MonoAssemblyName *name);
			void add_or_update_from_java (MonoDomain *domain, JNIEnv *env, jstring_array_wrapper &assemblies, jobjectArray assembliesBytes);
			void clear_for_domain (MonoDomain *domain);

		private:
			// TODO: replace with some sort of default data structure
			InMemoryAssemblyEntry **entries;
			int capacity;
			int length;

			InMemoryAssemblyEntry* find_entry (int domain_id)
			{
				for (int i = 0; i < length; i++) {
					auto entry = entries[i];
					if (entry->domain_id == domain_id)
						return entry;
				}
				return nullptr;
			}

			void add_or_replace_entry (InMemoryAssemblyEntry *new_entry)
			{
				for (int i = 0; i < length; i++) {
					auto entry = entries[i];
					if (entry->domain_id == new_entry->domain_id) {
						entries[i] = new_entry;
						delete entry;
						return;
					}
				}
				add_entry (new_entry);
			}

			void add_entry (InMemoryAssemblyEntry *entry)
			{
				if (length >= capacity) {
					capacity <<= 1;
					InMemoryAssemblyEntry **new_entries = new InMemoryAssemblyEntry*[capacity];
					memcpy ((void*)new_entries, entries, length);
					entries = new_entries;
				}
				entries[length++] = entry;
			}

			InMemoryAssemblyEntry* remove_entry (int domain_id)
			{
				for (int i = 0; i < length; i++) {
					auto entry = entries[i];
					if (entry->domain_id == domain_id) {
						for (int j = i; j < length - 1; j++)
							entries[j] = entries[j + 1];
						length--;
						return entry;
					}
				}
				return nullptr;
			}
	};
} } }

#endif