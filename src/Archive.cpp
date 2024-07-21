#include <fcntl.h>
#include <std/BytebufferType.hpp>

#include "ArchiveEntry.hpp"
#include "ArchiveFilters.hpp"
#include "ArchiveFormats.hpp"

static int copyData(struct archive *ar, struct archive *aw);

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *feralArchiveNew(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
		     const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>()) {
		vm.fail(args[1]->getLoc(),
			"expected an integer as the first argument to define open mode, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive *a = nullptr;
	int mode   = as<VarInt>(args[1])->get();
	if(mode == OM_READ) {
		a = archive_read_new();
	} else if(mode == OM_WRITE) {
		a = archive_write_new();
	}
	if(a == nullptr) {
		vm.fail(loc, "failed to init archive object, possibly invalid mode: ", mode);
		return nullptr;
	}
	return vm.makeVar<VarArchive>(loc, a, mode);
}

Var *feralArchiveOpen(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	VarArchive *ar = as<VarArchive>(args[0]);
	archive *a     = ar->get();

	if(!args[1]->is<VarStr>()) {
		vm.fail(args[1]->getLoc(), "expected a string file name to open as archive");
		return nullptr;
	}

	const std::string &name = as<VarStr>(args[1])->get();
	int code		= ARCHIVE_OK;

	if(ar->getMode() == OM_READ) {
		code = archive_read_open_filename(a, name.c_str(), 10240);
	} else if(ar->getMode() == OM_WRITE) {
		code = archive_write_open_filename(a, name.c_str());
	}
	if(code != ARCHIVE_OK) {
		vm.fail(loc, "failed to open archive in given mode");
		return nullptr;
	}
	return args[0];
}

Var *feralArchiveClose(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
		       const StringMap<AssnArgData> &assn_args)
{
	VarArchive *ar = as<VarArchive>(args[0]);
	if(ar->getMode() == OM_READ) archive_read_close(ar->get());
	else if(ar->getMode() == OM_WRITE) archive_write_close(ar->get());
	return args[0];
}

Var *feralArchiveWriteHeader(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
			     const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarArchiveEntry>()) {
		vm.fail(args[1]->getLoc(),
			"expected an archive entry for header, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_write_header(as<VarArchive>(args[0])->get(), as<VarArchiveEntry>(args[1])->get());
	return args[0];
}

Var *feralArchiveWriteData(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
			   const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarBytebuffer>()) {
		vm.fail(args[1]->getLoc(), "expected a bytebuffer for data to store, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_write_data(as<VarArchive>(args[0])->get(), as<VarBytebuffer>(args[1])->getBuf(),
			   as<VarBytebuffer>(args[1])->len());
	return args[0];
}

Var *feralArchiveAddFile(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
			 const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarStr>()) {
		vm.fail(args[1]->getLoc(), "expected a file name to write in archive");
		return nullptr;
	}
	VarArchive *ar		    = as<VarArchive>(args[0]);
	archive *a		    = ar->get();
	const std::string &filename = as<VarStr>(args[1])->get();

	struct stat st;
	stat(filename.c_str(), &st);
	archive_entry *entry = archive_entry_new();
	archive_entry_set_pathname(entry, filename.c_str());
	archive_entry_set_size(entry, st.st_size);
	archive_entry_set_filetype(entry, AE_IFREG);
	archive_entry_set_perm(entry, st.st_mode);
	archive_write_header(a, entry);
	int fdesc = open(filename.c_str(), O_RDONLY);
	char buff[8192];
	int len;
	while((len = read(fdesc, buff, sizeof(buff))) > 0) {
		archive_write_data(a, buff, len);
	}
	close(fdesc);
	archive_entry_free(entry);
	return args[0];
}

Var *feralArchiveExtract(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
			 const StringMap<AssnArgData> &assn_args)
{
	VarArchive *ar = as<VarArchive>(args[0]);
	archive *a     = ar->get();
	int extract_flags =
	ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
	archive *ext = archive_write_disk_new();
	archive_entry *entry;
	archive_write_disk_set_options(ext, extract_flags);
	archive_write_disk_set_standard_lookup(ext);
	int code = ARCHIVE_OK;
	for(;;) {
		code = archive_read_next_header(a, &entry);
		if(code == ARCHIVE_EOF) break;
		if(code < ARCHIVE_OK) {
			vm.fail(loc,
				"extract - read_next_header failed: ", archive_error_string(a));
			// goto end done by code < ARCHIVE_WARN
		}
		if(code < ARCHIVE_WARN) goto end;
		code = archive_write_header(ext, entry);
		if(code < ARCHIVE_OK) {
			vm.fail(loc, "extract - writer_header failed: ", archive_error_string(ext));
		} else if(archive_entry_size(entry) > 0) {
			code = copyData(a, ext);
			if(code < ARCHIVE_OK) {
				vm.fail(loc,
					"extract - copyData failed: ", archive_error_string(ext));
			}
			if(code < ARCHIVE_WARN) goto end;
		}
		code = archive_write_finish_entry(ext);
		if(code < ARCHIVE_OK) {
			vm.fail(loc,
				"extract - write_finish_entry failed: ", archive_error_string(ext));
		}
		if(code < ARCHIVE_WARN) goto end;
	}
end:
	archive_write_close(ext);
	archive_write_free(ext);
	return vm.makeVar<VarInt>(loc, code);
}

INIT_MODULE(Archive)
{
	VarModule *mod = vm.getCurrModule();

	mod->addNativeFn("newArchive", feralArchiveNew, 1);
	mod->addNativeFn("newEntry", feralArchiveEntryNew, 0);

	vm.addNativeTypeFn<VarArchive>(loc, "open", feralArchiveOpen, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "close", feralArchiveClose, 0);
	vm.addNativeTypeFn<VarArchive>(loc, "writeHeader", feralArchiveWriteHeader, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "writeData", feralArchiveWriteData, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "addFilter", feralArchiveApplyFilter, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "setFormat", feralArchiveApplyFormat, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "addFile", feralArchiveAddFile, 1);
	vm.addNativeTypeFn<VarArchive>(loc, "extract", feralArchiveExtract, 0);

	vm.addNativeTypeFn<VarArchiveEntry>(loc, "clear", feralArchiveEntryClear, 0);
	vm.addNativeTypeFn<VarArchiveEntry>(loc, "setPathname", feralArchiveEntrySetPathname, 1);
	vm.addNativeTypeFn<VarArchiveEntry>(loc, "setSize", feralArchiveEntrySetSize, 1);
	vm.addNativeTypeFn<VarArchiveEntry>(loc, "setFiletype", feralArchiveEntrySetFiletype, 1);
	vm.addNativeTypeFn<VarArchiveEntry>(loc, "setPerm", feralArchiveEntrySetPerm, 1);

	// register the archive types (registerType)
	vm.registerType<VarArchive>(loc, "Archive");
	vm.registerType<VarArchiveEntry>(loc, "ArchiveEntry");

	// enums

	// basic
	mod->addNativeVar("OPEN_READ", vm.makeVar<VarInt>(loc, OM_READ));
	mod->addNativeVar("OPEN_WRITE", vm.makeVar<VarInt>(loc, OM_WRITE));

	// filter
	mod->addNativeVar("FILTER_NONE", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_NONE));
	mod->addNativeVar("FILTER_GZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_GZIP));
	mod->addNativeVar("FILTER_BZIP2", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_BZIP2));
	mod->addNativeVar("FILTER_COMPRESS", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_COMPRESS));
	mod->addNativeVar("FILTER_PROGRAM", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_PROGRAM));
	mod->addNativeVar("FILTER_LZMA", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_LZMA));
	mod->addNativeVar("FILTER_XZ", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_XZ));
	mod->addNativeVar("FILTER_UU", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_UU));
	mod->addNativeVar("FILTER_RPM", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_RPM));
	mod->addNativeVar("FILTER_LZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_LZIP));
	mod->addNativeVar("FILTER_LRZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_LRZIP));
	mod->addNativeVar("FILTER_LZOP", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_LZOP));
	mod->addNativeVar("FILTER_GRZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_GRZIP));
	mod->addNativeVar("FILTER_LZ4", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_LZ4));
	mod->addNativeVar("FILTER_ZSTD", vm.makeVar<VarInt>(loc, ARCHIVE_FILTER_ZSTD));

	// formats
	mod->addNativeVar("FORMAT_CPIO", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_CPIO));
	mod->addNativeVar("FORMAT_TAR", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_TAR));
	mod->addNativeVar("FORMAT_TAR_USTAR", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_TAR_USTAR));
	mod->addNativeVar("FORMAT_TAR_PAX_INTERCHANGE",
			  vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE));
	mod->addNativeVar("FORMAT_TAR_PAX_RESTRICTED",
			  vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_TAR_PAX_RESTRICTED));
	mod->addNativeVar("FORMAT_TAR_GNUTAR", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_TAR_GNUTAR));
	mod->addNativeVar("FORMAT_ZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_ZIP));
	mod->addNativeVar("FORMAT_AR", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_AR));
	mod->addNativeVar("FORMAT_AR_BSD", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_AR_BSD));
	mod->addNativeVar("FORMAT_MTREE", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_MTREE));
	mod->addNativeVar("FORMAT_RAW", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_RAW));
	mod->addNativeVar("FORMAT_XAR", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_XAR));
	mod->addNativeVar("FORMAT_7ZIP", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_7ZIP));
	mod->addNativeVar("FORMAT_WARC", vm.makeVar<VarInt>(loc, ARCHIVE_FORMAT_WARC));

	mod->addNativeVar("E_IFREG", vm.makeVar<VarInt>(loc, AE_IFREG));
	mod->addNativeVar("E_IFDIR", vm.makeVar<VarInt>(loc, AE_IFDIR));
	mod->addNativeVar("E_IFCHR", vm.makeVar<VarInt>(loc, AE_IFCHR));
	mod->addNativeVar("E_IFBLK", vm.makeVar<VarInt>(loc, AE_IFBLK));
	mod->addNativeVar("E_IFIFO", vm.makeVar<VarInt>(loc, AE_IFIFO));
	mod->addNativeVar("E_IFLNK", vm.makeVar<VarInt>(loc, AE_IFLNK));
	mod->addNativeVar("E_IFSOCK", vm.makeVar<VarInt>(loc, AE_IFSOCK));

	return true;
}

static int copyData(struct archive *ar, struct archive *aw)
{
	int code;
	const void *buff;
	size_t size;
	la_int64_t offset;

	for(;;) {
		code = archive_read_data_block(ar, &buff, &size, &offset);
		if(code == ARCHIVE_EOF) return ARCHIVE_OK;
		if(code < ARCHIVE_OK) return code;
		code = archive_write_data_block(aw, buff, size, offset);
		if(code < ARCHIVE_OK) return code;
	}
}