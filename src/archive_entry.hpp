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

#include "../include/archive_type.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// Functions //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * feral_archive_entry_new( vm_state_t & vm, const fn_data_t & fd )
{
	archive_entry * ae = archive_entry_new();
	if( ae == nullptr ) {
		vm.fail( fd.src_id, fd.idx, "failed to init archive_entry object" );
		return nullptr;
	}
	return make< var_archive_entry_t >( ae );
}

var_base_t * feral_archive_entry_clear( vm_state_t & vm, const fn_data_t & fd )
{
	archive_entry_clear( ARCHIVE_ENTRY( fd.args[ 0 ] )->get() );
	return fd.args[ 0 ];
}

var_base_t * feral_archive_entry_set_pathname( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_str_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected a string for archive_entry pathname, found: '%s'",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	archive_entry_set_pathname( ARCHIVE_ENTRY( fd.args[ 0 ] )->get(), STR( fd.args[ 1 ] )->get().c_str() );
	return fd.args[ 0 ];
}

var_base_t * feral_archive_entry_set_size( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_int_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected an integer for archive_entry size, found: '%s'",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	archive_entry_set_size( ARCHIVE_ENTRY( fd.args[ 0 ] )->get(), mpz_get_ui( INT( fd.args[ 1 ] )->get() ) );
	return fd.args[ 0 ];
}

var_base_t * feral_archive_entry_set_filetype( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_int_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected an integer for archive_entry filetype, found: '%s'",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	archive_entry_set_filetype( ARCHIVE_ENTRY( fd.args[ 0 ] )->get(), mpz_get_ui( INT( fd.args[ 1 ] )->get() ) );
	return fd.args[ 0 ];
}

var_base_t * feral_archive_entry_set_perm( vm_state_t & vm, const fn_data_t & fd )
{
	if( !fd.args[ 1 ]->istype< var_int_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected an integer for archive_entry permissions, found: '%s'",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	archive_entry_set_size( ARCHIVE_ENTRY( fd.args[ 0 ] )->get(), mpz_get_ui( INT( fd.args[ 1 ] )->get() ) );
	return fd.args[ 0 ];
}