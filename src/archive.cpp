/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include <fcntl.h>

#include <feral/std/bytebuffer_type.hpp>

#include "archive_filters.hpp"
#include "archive_formats.hpp"

static int copy_data( struct archive * ar, struct archive * aw );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// Functions //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * feral_archive_new( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_int_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected an integer as the first argument to define open mode, found: '%s'",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	archive * a = nullptr;
	const int mode = mpz_get_si( INT( fd.args[ 1 ] )->get() );
	if( mode == OM_READ ) {
		a = archive_read_new();
	} else if( mode == OM_WRITE ) {
		a = archive_write_new();
	}
	if( a == nullptr ) {
		vm.fail( fd.src_id, fd.idx, "failed to init archive object, possibly invalid mode: %d", mode );
		return nullptr;
	}
	return make< var_archive_t >( a, mode );
}

var_base_t * feral_archive_open( vm_state_t & vm, const fn_data_t & fd )
{
	var_archive_t * ar = ARCHIVE( fd.args[ 0 ] );
	archive * a = ar->get();

	if( !fd.args[ 1 ]->istype< var_str_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(), "expected a string file name to open as archive" );
		return nullptr;
	}

	const std::string & name = STR( fd.args[ 1 ] )->get();
	int code = ARCHIVE_OK;

	if( ar->mode() == OM_READ ) {
		code = archive_read_open_filename( a, name.c_str(), 10240 );
	}	else if( ar->mode() == OM_WRITE ) {
		code = archive_write_open_filename( a, name.c_str() );
	}
	if( code != ARCHIVE_OK ) {
		vm.fail( fd.src_id, fd.idx, "failed to open archive in given mode" );
		return nullptr;
	}
	return fd.args[ 0 ];
}

var_base_t * feral_archive_close( vm_state_t & vm, const fn_data_t & fd )
{
	var_archive_t * ar = ARCHIVE( fd.args[ 0 ] );
	if( ar->mode() == OM_READ ) archive_read_close( ar->get() );
	else if( ar->mode() == OM_WRITE ) archive_write_close( ar->get() );
	return vm.nil;
}

// TODO: feral_archive_write (requires implementation of archive_entry class)

var_base_t * feral_archive_write_file( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_str_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(), "expected a file name to write in archive" );
		return nullptr;
	}
	var_archive_t * ar = ARCHIVE( fd.args[ 0 ] );
	archive * a = ar->get();
	const std::string & filename = STR( fd.args[ 1 ] )->get();

	struct stat st;
	stat( filename.c_str(), & st );
	archive_entry * entry = archive_entry_new();
	archive_entry_set_pathname( entry, filename.c_str() );
	archive_entry_set_size( entry, st.st_size );
	archive_entry_set_filetype( entry, AE_IFREG );
	archive_entry_set_perm( entry, st.st_mode );
	archive_write_header( a, entry );
	int fdesc = open( filename.c_str(), O_RDONLY );
	char buff[ 8192 ];
	int len;
	while( ( len = read( fdesc, buff, sizeof( buff ) ) ) > 0 ) {
		archive_write_data( a, buff, len );
	}
	close( fdesc );
	archive_entry_free( entry );
	return vm.tru;
}

var_base_t * feral_archive_extract( vm_state_t & vm, const fn_data_t & fd )
{
	var_archive_t * ar = ARCHIVE( fd.args[ 0 ] );
	archive * a = ar->get();
	int extract_flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
	archive * ext = archive_write_disk_new();
	archive_entry * entry;
	archive_write_disk_set_options( ext, extract_flags );
	archive_write_disk_set_standard_lookup( ext );
	int code = ARCHIVE_OK;
	for( ;; ) {
		code = archive_read_next_header( a, & entry );
		if( code == ARCHIVE_EOF ) break;
		if( code < ARCHIVE_OK ) {
			vm.fail( fd.src_id, fd.idx, "extract - read_next_header failed: %s", archive_error_string( a ) );
			// goto end done by code < ARCHIVE_WARN
		}
		if( code < ARCHIVE_WARN ) goto end;
		code = archive_write_header( ext, entry );
		if( code < ARCHIVE_OK ) {
			vm.fail( fd.src_id, fd.idx, "extract - writer_header failed: %s", archive_error_string( ext ) );
		} else if( archive_entry_size( entry ) > 0 ) {
			code = copy_data( a, ext );
			if( code < ARCHIVE_OK ) {
				vm.fail( fd.src_id, fd.idx, "extract - copy_data failed: %s", archive_error_string( ext ) );
			}
			if( code < ARCHIVE_WARN ) goto end;
		}
		code = archive_write_finish_entry( ext );
		if( code < ARCHIVE_OK ) {
			vm.fail( fd.src_id, fd.idx, "extract - write_finish_entry failed: %s", archive_error_string( ext ) );
		}
		if( code < ARCHIVE_WARN ) goto end;
	}
end:
	archive_write_close( ext );
	archive_write_free( ext );
	return make< var_int_t >( code );
}

INIT_MODULE( archive )
{
	var_src_t * src = vm.current_source();

	src->add_native_fn( "new", feral_archive_new, 1 );

	vm.add_native_typefn< var_archive_t >( "open",	     feral_archive_open,	 1, src_id, idx );
	vm.add_native_typefn< var_archive_t >( "close",	     feral_archive_close,	 0, src_id, idx );
	vm.add_native_typefn< var_archive_t >( "add_filter", feral_archive_apply_filter, 1, src_id, idx );
	vm.add_native_typefn< var_archive_t >( "set_format", feral_archive_apply_format, 1, src_id, idx );
	vm.add_native_typefn< var_archive_t >( "write_file", feral_archive_write_file,   1, src_id, idx );
	vm.add_native_typefn< var_archive_t >( "extract",    feral_archive_extract,	 0, src_id, idx );

	// register the archive type (register_type)
	vm.register_type< var_archive_t >( "archive", src_id, idx );

	// enums

	// basic
	src->add_native_var( "OPEN_READ",  make_all< var_int_t >( OM_READ,  src_id, idx ) );
	src->add_native_var( "OPEN_WRITE", make_all< var_int_t >( OM_WRITE, src_id, idx ) );

	// filter
	src->add_native_var( "FILTER_NONE",	make_all< var_int_t >( ARCHIVE_FILTER_NONE,	src_id, idx ) );
	src->add_native_var( "FILTER_GZIP",	make_all< var_int_t >( ARCHIVE_FILTER_GZIP,	src_id, idx ) );
	src->add_native_var( "FILTER_BZIP2",	make_all< var_int_t >( ARCHIVE_FILTER_BZIP2,	src_id, idx ) );
	src->add_native_var( "FILTER_COMPRESS", make_all< var_int_t >( ARCHIVE_FILTER_COMPRESS,	src_id, idx ) );
	src->add_native_var( "FILTER_PROGRAM",	make_all< var_int_t >( ARCHIVE_FILTER_PROGRAM,	src_id, idx ) );
	src->add_native_var( "FILTER_LZMA",	make_all< var_int_t >( ARCHIVE_FILTER_LZMA,	src_id, idx ) );
	src->add_native_var( "FILTER_XZ",	make_all< var_int_t >( ARCHIVE_FILTER_XZ,	src_id, idx ) );
	src->add_native_var( "FILTER_UU",	make_all< var_int_t >( ARCHIVE_FILTER_UU,	src_id, idx ) );
	src->add_native_var( "FILTER_RPM",	make_all< var_int_t >( ARCHIVE_FILTER_RPM,	src_id, idx ) );
	src->add_native_var( "FILTER_LZIP",	make_all< var_int_t >( ARCHIVE_FILTER_LZIP,	src_id, idx ) );
	src->add_native_var( "FILTER_LRZIP",	make_all< var_int_t >( ARCHIVE_FILTER_LRZIP,	src_id, idx ) );
	src->add_native_var( "FILTER_LZOP",	make_all< var_int_t >( ARCHIVE_FILTER_LZOP,	src_id, idx ) );
	src->add_native_var( "FILTER_GRZIP",	make_all< var_int_t >( ARCHIVE_FILTER_GRZIP,	src_id, idx ) );
	src->add_native_var( "FILTER_LZ4",	make_all< var_int_t >( ARCHIVE_FILTER_LZ4,	src_id, idx ) );
	src->add_native_var( "FILTER_ZSTD",	make_all< var_int_t >( ARCHIVE_FILTER_ZSTD,	src_id, idx ) );

	// formats
	src->add_native_var( "FORMAT_CPIO",	make_all< var_int_t >( ARCHIVE_FORMAT_CPIO,	src_id, idx ) );
	src->add_native_var( "FORMAT_TAR",	make_all< var_int_t >( ARCHIVE_FORMAT_TAR,	src_id, idx ) );
	src->add_native_var( "FORMAT_TAR_USTAR", make_all< var_int_t >( ARCHIVE_FORMAT_TAR_USTAR, src_id, idx ) );
	src->add_native_var( "FORMAT_TAR_PAX_INTERCHANGE", make_all< var_int_t >( ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE, src_id, idx ) );
	src->add_native_var( "FORMAT_TAR_PAX_RESTRICTED", make_all< var_int_t >( ARCHIVE_FORMAT_TAR_PAX_RESTRICTED, src_id, idx ) );
	src->add_native_var( "FORMAT_TAR_GNUTAR", make_all< var_int_t >( ARCHIVE_FORMAT_TAR_GNUTAR, src_id, idx ) );
	src->add_native_var( "FORMAT_ZIP",	make_all< var_int_t >( ARCHIVE_FORMAT_ZIP,	src_id, idx ) );
	src->add_native_var( "FORMAT_AR",	make_all< var_int_t >( ARCHIVE_FORMAT_AR,	src_id, idx ) );
	src->add_native_var( "FORMAT_AR_BSD",	make_all< var_int_t >( ARCHIVE_FORMAT_AR_BSD,	src_id, idx ) );
	src->add_native_var( "FORMAT_MTREE",	make_all< var_int_t >( ARCHIVE_FORMAT_MTREE,	src_id, idx ) );
	src->add_native_var( "FORMAT_RAW",	make_all< var_int_t >( ARCHIVE_FORMAT_RAW,	src_id, idx ) );
	src->add_native_var( "FORMAT_XAR",	make_all< var_int_t >( ARCHIVE_FORMAT_XAR,	src_id, idx ) );
	src->add_native_var( "FORMAT_7ZIP",	make_all< var_int_t >( ARCHIVE_FORMAT_7ZIP,	src_id, idx ) );
	src->add_native_var( "FORMAT_WARC",	make_all< var_int_t >( ARCHIVE_FORMAT_WARC,	src_id, idx ) );

	return true;
}

static int copy_data( struct archive * ar, struct archive * aw )
{
	int code;
	const void * buff;
	size_t size;
	la_int64_t offset;

	for( ;; ) {
		code = archive_read_data_block( ar, & buff, & size, & offset );
		if( code == ARCHIVE_EOF ) return ARCHIVE_OK;
		if( code < ARCHIVE_OK ) return code;
		code = archive_write_data_block( aw, buff, size, offset );
		if( code < ARCHIVE_OK ) return code;
	}
}