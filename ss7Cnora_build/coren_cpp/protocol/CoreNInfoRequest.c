/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "MsgProto1"
 * 	found in "/opt/svyazcom/ss7_dmp_cnora/ss7_dmp/coren_cpp/protocol/coren.asn1"
 * 	`asn1c -fwide-types`
 */

#include "CoreNInfoRequest.h"

static asn_TYPE_member_t asn_MBR_CoreNInfoRequest_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CoreNInfoRequest, code),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CoreNInfoCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"code"
		},
};
static const ber_tlv_tag_t asn_DEF_CoreNInfoRequest_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CoreNInfoRequest_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* code */
};
static asn_SEQUENCE_specifics_t asn_SPC_CoreNInfoRequest_specs_1 = {
	sizeof(struct CoreNInfoRequest),
	offsetof(struct CoreNInfoRequest, _asn_ctx),
	asn_MAP_CoreNInfoRequest_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CoreNInfoRequest = {
	"CoreNInfoRequest",
	"CoreNInfoRequest",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CoreNInfoRequest_tags_1,
	sizeof(asn_DEF_CoreNInfoRequest_tags_1)
		/sizeof(asn_DEF_CoreNInfoRequest_tags_1[0]), /* 1 */
	asn_DEF_CoreNInfoRequest_tags_1,	/* Same as above */
	sizeof(asn_DEF_CoreNInfoRequest_tags_1)
		/sizeof(asn_DEF_CoreNInfoRequest_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CoreNInfoRequest_1,
	1,	/* Elements count */
	&asn_SPC_CoreNInfoRequest_specs_1	/* Additional specs */
};

