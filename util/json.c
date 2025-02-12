// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <util/json.h>
#include <json-c/json.h>
#include <json-c/printbuf.h>

/* adapted from mdadm::human_size_brief() */
static int display_size(struct json_object *jobj, struct printbuf *pbuf,
		int level, int flags)
{
	unsigned long long bytes = json_object_get_int64(jobj);
	static char buf[128];
	int c;

	/*
	 * We convert bytes to either centi-M{ega,ibi}bytes or
	 * centi-G{igi,ibi}bytes, with appropriate rounding, and then print
	 * 1/100th of those as a decimal.  We allow upto 2048Megabytes before
	 * converting to gigabytes, as that shows more precision and isn't too
	 * large a number.  Terabytes are not yet handled.
	 *
	 * If prefix == IEC, we mean prefixes like kibi,mebi,gibi etc.
	 * If prefix == JEDEC, we mean prefixes like kilo,mega,giga etc.
	 */

	if (bytes < 5000*1024)
		snprintf(buf, sizeof(buf), "%lld", bytes);
	else {
		/* IEC */
		if (bytes < 2*1024LL*1024LL*1024LL) {
			long cMiB = (bytes * 200LL / (1LL<<20) +1) /2;

			c = snprintf(buf, sizeof(buf), "\"%ld.%02ld MiB",
					cMiB/100 , cMiB % 100);
		} else {
			long cGiB = (bytes * 200LL / (1LL<<30) +1) /2;

			c = snprintf(buf, sizeof(buf), "\"%ld.%02ld GiB",
					cGiB/100 , cGiB % 100);
		}

		/* JEDEC */
		if (bytes < 2*1024LL*1024LL*1024LL) {
			long cMB  = (bytes / (1000000LL / 200LL) + 1) / 2;

			snprintf(buf + c, sizeof(buf) - c, " (%ld.%02ld MB)\"",
					cMB/100, cMB % 100);
		} else {
			long cGB  = (bytes / (1000000000LL/200LL) + 1) / 2;

			snprintf(buf + c, sizeof(buf) - c, " (%ld.%02ld GB)\"",
					cGB/100 , cGB % 100);
		}
	}

	return printbuf_memappend(pbuf, buf, strlen(buf));
}

static int display_hex(struct json_object *jobj, struct printbuf *pbuf,
		int level, int flags)
{
	unsigned long long val = json_object_get_int64(jobj);
	static char buf[32];

	snprintf(buf, sizeof(buf), "\"%#llx\"", val);
	return printbuf_memappend(pbuf, buf, strlen(buf));
}

struct json_object *util_json_object_size(unsigned long long size,
		unsigned long flags)
{
	struct json_object *jobj = json_object_new_int64(size);

	if (jobj && (flags & UTIL_JSON_HUMAN))
		json_object_set_serializer(jobj, display_size, NULL, NULL);
	return jobj;
}

struct json_object *util_json_object_hex(unsigned long long val,
		unsigned long flags)
{
	struct json_object *jobj = json_object_new_int64(val);

	if (jobj && (flags & UTIL_JSON_HUMAN))
		json_object_set_serializer(jobj, display_hex, NULL, NULL);
	return jobj;
}

void util_display_json_array(FILE *f_out, struct json_object *jarray,
		unsigned long flags)
{
	int len = json_object_array_length(jarray);
	int jflag = JSON_C_TO_STRING_PRETTY;

	if (json_object_array_length(jarray) > 1 || !(flags & UTIL_JSON_HUMAN))
		fprintf(f_out, "%s\n", json_object_to_json_string_ext(jarray, jflag));
	else if (len) {
		struct json_object *jobj;

		jobj = json_object_array_get_idx(jarray, 0);
		fprintf(f_out, "%s\n", json_object_to_json_string_ext(jobj, jflag));
	}
	json_object_put(jarray);
}
