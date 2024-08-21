#include <boot/multiboot2.h>

#include <stddef.h>

struct mb2_tag_base *mb2_find(const struct mb2_info *info, u32 type)
{
	const struct mb2_tag_base *tag =
	    (const struct mb2_tag_base *)info->tags;

	for (; tag->type != MB2_TAG_TYPE_END; tag = MB2_NEXT_TAG(tag)) {
		if (tag->type == type) {
			return (struct mb2_tag_base *)tag;
		}
	}
	return NULL;
}
