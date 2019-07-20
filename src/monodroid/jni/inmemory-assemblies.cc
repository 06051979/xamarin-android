
#include "inmemory-assemblies.h"
#include "globals.h"

extern "C" {
#include "java-interop-util.h"
#include "logger.h"
}

namespace xamarin { namespace android { namespace internal {

void
InMemoryAssemblies::add_or_update_from_java (MonoDomain *domain, JNIEnv *env, jstring_array_wrapper &assemblies, jobjectArray assembliesBytes)
{
	if (assembliesBytes == nullptr)
		return;

	jsize len = env->GetArrayLength (assembliesBytes);
	char **names = new char*[len];
	char **assemblies_bytes = new char*[len];
	int *assemblies_bytes_len = new int[len];

	for (int index = 0; index < len; index++) {
		jboolean is_copy;
		jbyteArray assembly_byte_array = reinterpret_cast <jbyteArray> (env->GetObjectArrayElement (assembliesBytes, index));
		jsize bytes_len = env->GetArrayLength (assembly_byte_array);
		jbyte *bytes = env->GetByteArrayElements (assembly_byte_array, &is_copy);
		jstring_wrapper &assembly = assemblies [index];

		names[index] = utils.strdup_new (assembly.get_cstr ());
		assemblies_bytes_len[index] = (int)bytes_len;
		assemblies_bytes[index] = new char[bytes_len];
		memcpy ((void*)assemblies_bytes[index], bytes, bytes_len);

		env->ReleaseByteArrayElements (assembly_byte_array, bytes, JNI_ABORT);
	}

	InMemoryAssemblyEntry *new_entry = new InMemoryAssemblyEntry;
	new_entry->domain_id = monoFunctions.domain_get_id (domain);
	new_entry->assemblies_count = len;
	new_entry->names = names;
	new_entry->assemblies_bytes = assemblies_bytes;
	new_entry->assemblies_bytes_len = assemblies_bytes_len;

	add_or_replace_entry (new_entry);
}

MonoAssembly*
InMemoryAssemblies::load_assembly_from_memory (MonoDomain *domain, MonoAssemblyName *name)
{
	int domain_id = monoFunctions.domain_get_id (domain);
	InMemoryAssemblyEntry *entry = find_entry (domain_id);
	if (entry == nullptr)
		return nullptr;

	const char *asm_name = monoFunctions.assembly_name_get_name (name);
	int asm_count = entry->assemblies_count;

	for (int i = 0; i < asm_count; i++) {
		const char *entry_name = entry->names[i];
		const char *entry_bytes = entry->assemblies_bytes[i];
		const int entry_bytes_len = entry->assemblies_bytes_len[i];

		if (strcmp (asm_name, entry_name) != 0)
			continue;

		// There is unfortunately no public unmanaged API to do proper in-memory
		// loading (it would require access to the MonoAssemblyLoadRequest API)
		MonoClass *assembly_klass = utils.monodroid_get_class_from_name (domain, "mscorlib", "System.Reflection", "Assembly");
		MonoClass *byte_klass = monoFunctions.get_byte_class ();
		// Use the variant with 3 parameters so that we always get the first argument being a byte[]
		// (the two last don't matter since we pass null anyway)
		MonoMethod *assembly_load_method = monoFunctions.class_get_method_from_name (assembly_klass, "Load", 3);
		MonoArray *byteArray = monoFunctions.array_new (domain, byte_klass, entry_bytes_len);
		monoFunctions.value_copy_array (byteArray, 0, (void*)entry_bytes, entry_bytes_len);

		void *args[3];
		args[0] = byteArray;
		args[1] = nullptr;
		args[2] = nullptr;
		MonoObject *res = utils.monodroid_runtime_invoke (domain, assembly_load_method, nullptr, args, nullptr);
		MonoAssembly *mono_assembly = monoFunctions.reflection_assembly_get_assembly (res);

		log_info (LOG_DEFAULT, "Loaded %s from memory in domain %d", entry_name, domain_id);

		return mono_assembly;
	}

	return nullptr;
}

void
InMemoryAssemblies::clear_for_domain (MonoDomain *domain)
{
	int domain_id = monoFunctions.domain_get_id (domain);
	InMemoryAssemblyEntry *entry = remove_entry (domain_id);
	if (entry != nullptr)
		delete entry;
}

} } }