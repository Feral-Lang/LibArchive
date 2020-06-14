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

#include "../include/archive_type.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// Functions //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * feral_archive_apply_format( vm_state_t & vm, const fn_data_t & fd )
{
	var_archive_t * ar = ARCHIVE( fd.args[ 0 ] );
	archive * a = ar->get();
	if( !fd.args[ 1 ]->istype< var_int_t >() ) {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "expected format id to be of type 'int', found: %s",
			 vm.type_name( fd.args[ 1 ] ).c_str() );
		return nullptr;
	}
	// list available here: https://github.com/libarchive/libarchive/blob/c400064a1c63d122340d09d8ce3f671d4cf24b6e/libarchive/archive.h#L312
	int format = mpz_get_si( INT( fd.args[ 1 ] )->get() );

	switch( format ) {
	case ARCHIVE_FORMAT_CPIO: {
		if( ar->mode() == OM_READ ) archive_read_support_format_cpio( a );
		else archive_write_set_format_cpio( a );
		break;
	}
	case ARCHIVE_FORMAT_TAR_USTAR:
	case ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE:
	case ARCHIVE_FORMAT_TAR_PAX_RESTRICTED:
	case ARCHIVE_FORMAT_TAR_GNUTAR:
	case ARCHIVE_FORMAT_TAR: {
		if( ar->mode() == OM_READ ) archive_read_support_format_tar( a );
		else {
			if( format == ARCHIVE_FORMAT_TAR_USTAR ) {
				archive_write_set_format_ustar( a );
			} else if( format == ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE ) {
				archive_write_set_format_pax( a );
			} else if( format == ARCHIVE_FORMAT_TAR_PAX_RESTRICTED ) {
				archive_write_set_format_pax_restricted( a );
			} else if( format == ARCHIVE_FORMAT_TAR_GNUTAR ) {
				archive_write_set_format_gnutar( a );
			} else {
				vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
					 "required specific writing format for tar: one of "
					 "FORMAT_TAR_{USTAR, PAX_INTERCHANGE, PAX_RESTRICTED, GNUTAR}" );
				return nullptr;
			}
		}
		break;
	}
	case ARCHIVE_FORMAT_ZIP: {
		if( ar->mode() == OM_READ ) archive_read_support_format_zip( a );
		else archive_write_set_format_zip( a );
		break;
	}
	case ARCHIVE_FORMAT_AR:
	case ARCHIVE_FORMAT_AR_BSD: {
		if( ar->mode() == OM_READ ) archive_read_support_format_ar( a );
		else archive_write_set_format_ar_bsd( a );
		break;
	}
	case ARCHIVE_FORMAT_MTREE: {
		if( ar->mode() == OM_READ ) archive_read_support_format_mtree( a );
		else archive_write_set_format_mtree( a );
		break;
	}
	case ARCHIVE_FORMAT_RAW: {
		if( ar->mode() == OM_READ ) archive_read_support_format_raw( a );
		else archive_write_set_format_raw( a );
		break;
	}
	case ARCHIVE_FORMAT_XAR: {
		if( ar->mode() == OM_READ ) archive_read_support_format_xar( a );
		else archive_write_set_format_xar( a );
		break;
	}
	case ARCHIVE_FORMAT_7ZIP: {
		if( ar->mode() == OM_READ ) archive_read_support_format_7zip( a );
		else archive_write_set_format_7zip( a );
		break;
	}
	case ARCHIVE_FORMAT_WARC: {
		if( ar->mode() == OM_READ ) archive_read_support_format_warc( a );
		else archive_write_set_format_warc( a );
		break;
	}
	default: {
		vm.fail( fd.args[ 1 ]->src_id(), fd.args[ 1 ]->idx(),
			 "invalid format found: %d", format );
		return nullptr;
	}
	}
	return fd.args[ 0 ];
}