
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

		names[index] = utils.get_file_name_without_extension (assembly.get_cstr ());
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
	MonoAssembly *result = nullptr;
	MonoDomain *current = monoFunctions.domain_get ();
	if (current != domain)
		monoFunctions.domain_set (domain, FALSE);

	for (int i = 0; i < asm_count; i++) {
		const char *entry_name = entry->names[i];
		const char *entry_bytes = entry->assemblies_bytes[i];
		const int entry_bytes_len = entry->assemblies_bytes_len[i];

		if (strcmp (asm_name, entry_name) == 0) {
			MonoImageOpenStatus status;
			const char *image_name = monodroid_strdup_printf ("%s_%d", entry_name, domain_id);
			MonoImage *image = monoFunctions.image_open_from_data_with_name ((char*)entry_bytes, entry_bytes_len, /*need_copy:*/true, &status, /*refonly:*/false, image_name);
			if (!image || status != MonoImageOpenStatus::MONO_IMAGE_OK) {
				if (image)
					monoFunctions.image_close (image);
				log_error (LOG_DEFAULT, "Couldn't load in-memory image for assembly %s, domain %d", entry_name, domain_id);
				goto Cleanup;
			}

			// If image already has an associated assembly it means it was loaded earlier
			MonoAssembly *mono_assembly = monoFunctions.image_get_assembly (image);
			if (mono_assembly) {
				monoFunctions.image_close (image);
				log_info (LOG_DEFAULT, "Skipping %s that was already loaded for domain %d", entry_name, domain_id);
				result = mono_assembly;
				goto Cleanup;
			}

			mono_assembly = monoFunctions.assembly_load_from_full (image, entry_name, &status, /*refonly:*/false);
			monoFunctions.image_close (image);
			if (!mono_assembly) {
				log_error (LOG_DEFAULT, "Couldn't load in-memory assembly %s", entry_name);
				goto Cleanup;
			}

			log_info (LOG_DEFAULT, "Loaded %s from memory in domain %d", entry_name, domain_id);

			result = mono_assembly;
		}
	}

Cleanup:
	if (current != domain)
		monoFunctions.domain_set (current, FALSE);

	return result;
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