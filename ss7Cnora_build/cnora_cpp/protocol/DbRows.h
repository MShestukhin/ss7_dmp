/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "CnoraProtocol"
 * 	found in "/opt/svyazcom/ss7_dmp_cnora/ss7_dmp/cnora_cpp/protocol/cnora.asn1"
 * 	`asn1c -fwide-types`
 */

#ifndef	_DbRows_H_
#define	_DbRows_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct DbRow;

/* DbRows */
typedef struct DbRows {
	A_SEQUENCE_OF(struct DbRow) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DbRows_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DbRows;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "DbRow.h"

#endif	/* _DbRows_H_ */
#include <asn_internal.h>
