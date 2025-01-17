#include <stdatomic.h>
#include <kernel/kernel.h>
#include <kernel/tty.h> //TTY will always be available
#include <kernel/ps2.h>
#include <kernel/printk.h>
#include <kernel/platform.h>
#include <kernel/arch/i386/platform.h>
#include <kernel/arch/i386/io.h>

void reset(void)
{
	while (!ps2_cansend())
		continue;
	ps2_send_cmd(PS2_CMD_RESET);
	idle();
}

void halt(void)
{
	__asm__ volatile("hlt");
}

static void dump_stacktrace_ebp(const void *ebpp)
{
	const u32* ebp = ebpp;
	unsigned level = 0;

	while (ebp) {
		printk("%03d: 0x%08lx\n", level, *(ebp + 1));
		ebp = (const u32*) *ebp;
		level += 1;
	}
}

void dump_stacktrace_at(const struct cpu_state *state)
{
	dump_stacktrace_ebp((void*) (uintptr_t)state->ebp);
}

void dump_stacktrace(void)
{
	u32 *ebp;
	__asm__ volatile("mov %0,%%ebp" : "=r"(ebp));
	dump_stacktrace_ebp(ebp);
}

#define DUMP_REGISTER(reg)                                         \
	do {                                                       \
		u32 __val;                                         \
		__asm__ volatile("mov %0," reg : "=r"(__val));     \
		printk("%s: 0x%08lx (%lu)\n", reg, __val, __val); \
	} while (0)

void dump_registers(void)
{
	DUMP_REGISTER("eax");
	DUMP_REGISTER("ebx");
	DUMP_REGISTER("ecx");
	DUMP_REGISTER("edx");
	DUMP_REGISTER("esi");
	DUMP_REGISTER("edi");
	DUMP_REGISTER("esi");
	DUMP_REGISTER("ebp");
}

void dump_state(const struct cpu_state *state)
{
	DUMP_REGISTER("cr0");
	DUMP_REGISTER("cr2");
	DUMP_REGISTER("cr3");
	DUMP_REGISTER("cr4");

	printk("vec_num: 0x%08lx ", state->vec_num);
	printk("err_code: 0x%08lx\n", state->err_code);

	printk("edi: 0x%08lx ", state->edi);
	printk("esi: 0x%08lx ", state->esi);
	printk("ebp: 0x%08lx\n", state->ebp);
	printk("esp: 0x%08lx ", state->esp);
	printk("ebx: 0x%08lx ", state->ebx);
	printk("edx: 0x%08lx\n", state->edx);
	printk("ecx: 0x%08lx ", state->ecx);
	printk("eax: 0x%08lx ", state->eax);
	printk("eip: 0x%08lx\n", state->eip);
	printk("cs: 0x%04hx\n", state->cs);

}

static atomic_bool _irqs_enabled = false;

void disable_irqs(void)
{
	__asm__ volatile("cli");
}

void enable_irqs(void)
{
	if (atomic_load(&_irqs_enabled)) {
		__asm__ volatile("sti");
	}
}

void __enable_irqs(void)
{
	atomic_store(&_irqs_enabled, true);
	enable_irqs();
}

void short_wait(void)
{
	io_wait();
}

extern char _user_text_start[];
extern char _user_text_end[];
bool is_from_uaccess(const struct cpu_state *state)
{
	return (char *)state->eip >= _user_text_start &&
	       (char *)state->eip < _user_text_end;
}

bool is_from_userspace(const struct cpu_state *state)
{
	/* TODO this doesn't seem that corrent, the requested privilege level
	 * could be diferrent perhaps */
	return state->cs != I386_KERNEL_CODE_SELECTOR;
}

void do_irq(u8 irqn)
{
	switch (irqn) {
	case 0:
		__asm__ volatile("int 0");
		break;
	case 1:
		__asm__ volatile("int 1");
		break;
	case 2:
		__asm__ volatile("int 2");
		break;
	case 3:
		__asm__ volatile("int 3");
		break;
	case 4:
		__asm__ volatile("int 4");
		break;
	case 5:
		__asm__ volatile("int 5");
		break;
	case 6:
		__asm__ volatile("int 6");
		break;
	case 7:
		__asm__ volatile("int 7");
		break;
	case 8:
		__asm__ volatile("int 8");
		break;
	case 9:
		__asm__ volatile("int 9");
		break;
	case 10:
		__asm__ volatile("int 10");
		break;
	case 11:
		__asm__ volatile("int 11");
		break;
	case 12:
		__asm__ volatile("int 12");
		break;
	case 13:
		__asm__ volatile("int 13");
		break;
	case 14:
		__asm__ volatile("int 14");
		break;
	case 15:
		__asm__ volatile("int 15");
		break;
	case 16:
		__asm__ volatile("int 16");
		break;
	case 17:
		__asm__ volatile("int 17");
		break;
	case 18:
		__asm__ volatile("int 18");
		break;
	case 19:
		__asm__ volatile("int 19");
		break;
	case 20:
		__asm__ volatile("int 20");
		break;
	case 21:
		__asm__ volatile("int 21");
		break;
	case 22:
		__asm__ volatile("int 22");
		break;
	case 23:
		__asm__ volatile("int 23");
		break;
	case 24:
		__asm__ volatile("int 24");
		break;
	case 25:
		__asm__ volatile("int 25");
		break;
	case 26:
		__asm__ volatile("int 26");
		break;
	case 27:
		__asm__ volatile("int 27");
		break;
	case 28:
		__asm__ volatile("int 28");
		break;
	case 29:
		__asm__ volatile("int 29");
		break;
	case 30:
		__asm__ volatile("int 30");
		break;
	case 31:
		__asm__ volatile("int 31");
		break;
	case 32:
		__asm__ volatile("int 32");
		break;
	case 33:
		__asm__ volatile("int 33");
		break;
	case 34:
		__asm__ volatile("int 34");
		break;
	case 35:
		__asm__ volatile("int 35");
		break;
	case 36:
		__asm__ volatile("int 36");
		break;
	case 37:
		__asm__ volatile("int 37");
		break;
	case 38:
		__asm__ volatile("int 38");
		break;
	case 39:
		__asm__ volatile("int 39");
		break;
	case 40:
		__asm__ volatile("int 40");
		break;
	case 41:
		__asm__ volatile("int 41");
		break;
	case 42:
		__asm__ volatile("int 42");
		break;
	case 43:
		__asm__ volatile("int 43");
		break;
	case 44:
		__asm__ volatile("int 44");
		break;
	case 45:
		__asm__ volatile("int 45");
		break;
	case 46:
		__asm__ volatile("int 46");
		break;
	case 47:
		__asm__ volatile("int 47");
		break;
	case 48:
		__asm__ volatile("int 48");
		break;
	case 49:
		__asm__ volatile("int 49");
		break;
	case 50:
		__asm__ volatile("int 50");
		break;
	case 51:
		__asm__ volatile("int 51");
		break;
	case 52:
		__asm__ volatile("int 52");
		break;
	case 53:
		__asm__ volatile("int 53");
		break;
	case 54:
		__asm__ volatile("int 54");
		break;
	case 55:
		__asm__ volatile("int 55");
		break;
	case 56:
		__asm__ volatile("int 56");
		break;
	case 57:
		__asm__ volatile("int 57");
		break;
	case 58:
		__asm__ volatile("int 58");
		break;
	case 59:
		__asm__ volatile("int 59");
		break;
	case 60:
		__asm__ volatile("int 60");
		break;
	case 61:
		__asm__ volatile("int 61");
		break;
	case 62:
		__asm__ volatile("int 62");
		break;
	case 63:
		__asm__ volatile("int 63");
		break;
	case 64:
		__asm__ volatile("int 64");
		break;
	case 65:
		__asm__ volatile("int 65");
		break;
	case 66:
		__asm__ volatile("int 66");
		break;
	case 67:
		__asm__ volatile("int 67");
		break;
	case 68:
		__asm__ volatile("int 68");
		break;
	case 69:
		__asm__ volatile("int 69");
		break;
	case 70:
		__asm__ volatile("int 70");
		break;
	case 71:
		__asm__ volatile("int 71");
		break;
	case 72:
		__asm__ volatile("int 72");
		break;
	case 73:
		__asm__ volatile("int 73");
		break;
	case 74:
		__asm__ volatile("int 74");
		break;
	case 75:
		__asm__ volatile("int 75");
		break;
	case 76:
		__asm__ volatile("int 76");
		break;
	case 77:
		__asm__ volatile("int 77");
		break;
	case 78:
		__asm__ volatile("int 78");
		break;
	case 79:
		__asm__ volatile("int 79");
		break;
	case 80:
		__asm__ volatile("int 80");
		break;
	case 81:
		__asm__ volatile("int 81");
		break;
	case 82:
		__asm__ volatile("int 82");
		break;
	case 83:
		__asm__ volatile("int 83");
		break;
	case 84:
		__asm__ volatile("int 84");
		break;
	case 85:
		__asm__ volatile("int 85");
		break;
	case 86:
		__asm__ volatile("int 86");
		break;
	case 87:
		__asm__ volatile("int 87");
		break;
	case 88:
		__asm__ volatile("int 88");
		break;
	case 89:
		__asm__ volatile("int 89");
		break;
	case 90:
		__asm__ volatile("int 90");
		break;
	case 91:
		__asm__ volatile("int 91");
		break;
	case 92:
		__asm__ volatile("int 92");
		break;
	case 93:
		__asm__ volatile("int 93");
		break;
	case 94:
		__asm__ volatile("int 94");
		break;
	case 95:
		__asm__ volatile("int 95");
		break;
	case 96:
		__asm__ volatile("int 96");
		break;
	case 97:
		__asm__ volatile("int 97");
		break;
	case 98:
		__asm__ volatile("int 98");
		break;
	case 99:
		__asm__ volatile("int 99");
		break;
	case 100:
		__asm__ volatile("int 100");
		break;
	case 101:
		__asm__ volatile("int 101");
		break;
	case 102:
		__asm__ volatile("int 102");
		break;
	case 103:
		__asm__ volatile("int 103");
		break;
	case 104:
		__asm__ volatile("int 104");
		break;
	case 105:
		__asm__ volatile("int 105");
		break;
	case 106:
		__asm__ volatile("int 106");
		break;
	case 107:
		__asm__ volatile("int 107");
		break;
	case 108:
		__asm__ volatile("int 108");
		break;
	case 109:
		__asm__ volatile("int 109");
		break;
	case 110:
		__asm__ volatile("int 110");
		break;
	case 111:
		__asm__ volatile("int 111");
		break;
	case 112:
		__asm__ volatile("int 112");
		break;
	case 113:
		__asm__ volatile("int 113");
		break;
	case 114:
		__asm__ volatile("int 114");
		break;
	case 115:
		__asm__ volatile("int 115");
		break;
	case 116:
		__asm__ volatile("int 116");
		break;
	case 117:
		__asm__ volatile("int 117");
		break;
	case 118:
		__asm__ volatile("int 118");
		break;
	case 119:
		__asm__ volatile("int 119");
		break;
	case 120:
		__asm__ volatile("int 120");
		break;
	case 121:
		__asm__ volatile("int 121");
		break;
	case 122:
		__asm__ volatile("int 122");
		break;
	case 123:
		__asm__ volatile("int 123");
		break;
	case 124:
		__asm__ volatile("int 124");
		break;
	case 125:
		__asm__ volatile("int 125");
		break;
	case 126:
		__asm__ volatile("int 126");
		break;
	case 127:
		__asm__ volatile("int 127");
		break;
	case 128:
		__asm__ volatile("int 128");
		break;
	case 129:
		__asm__ volatile("int 129");
		break;
	case 130:
		__asm__ volatile("int 130");
		break;
	case 131:
		__asm__ volatile("int 131");
		break;
	case 132:
		__asm__ volatile("int 132");
		break;
	case 133:
		__asm__ volatile("int 133");
		break;
	case 134:
		__asm__ volatile("int 134");
		break;
	case 135:
		__asm__ volatile("int 135");
		break;
	case 136:
		__asm__ volatile("int 136");
		break;
	case 137:
		__asm__ volatile("int 137");
		break;
	case 138:
		__asm__ volatile("int 138");
		break;
	case 139:
		__asm__ volatile("int 139");
		break;
	case 140:
		__asm__ volatile("int 140");
		break;
	case 141:
		__asm__ volatile("int 141");
		break;
	case 142:
		__asm__ volatile("int 142");
		break;
	case 143:
		__asm__ volatile("int 143");
		break;
	case 144:
		__asm__ volatile("int 144");
		break;
	case 145:
		__asm__ volatile("int 145");
		break;
	case 146:
		__asm__ volatile("int 146");
		break;
	case 147:
		__asm__ volatile("int 147");
		break;
	case 148:
		__asm__ volatile("int 148");
		break;
	case 149:
		__asm__ volatile("int 149");
		break;
	case 150:
		__asm__ volatile("int 150");
		break;
	case 151:
		__asm__ volatile("int 151");
		break;
	case 152:
		__asm__ volatile("int 152");
		break;
	case 153:
		__asm__ volatile("int 153");
		break;
	case 154:
		__asm__ volatile("int 154");
		break;
	case 155:
		__asm__ volatile("int 155");
		break;
	case 156:
		__asm__ volatile("int 156");
		break;
	case 157:
		__asm__ volatile("int 157");
		break;
	case 158:
		__asm__ volatile("int 158");
		break;
	case 159:
		__asm__ volatile("int 159");
		break;
	case 160:
		__asm__ volatile("int 160");
		break;
	case 161:
		__asm__ volatile("int 161");
		break;
	case 162:
		__asm__ volatile("int 162");
		break;
	case 163:
		__asm__ volatile("int 163");
		break;
	case 164:
		__asm__ volatile("int 164");
		break;
	case 165:
		__asm__ volatile("int 165");
		break;
	case 166:
		__asm__ volatile("int 166");
		break;
	case 167:
		__asm__ volatile("int 167");
		break;
	case 168:
		__asm__ volatile("int 168");
		break;
	case 169:
		__asm__ volatile("int 169");
		break;
	case 170:
		__asm__ volatile("int 170");
		break;
	case 171:
		__asm__ volatile("int 171");
		break;
	case 172:
		__asm__ volatile("int 172");
		break;
	case 173:
		__asm__ volatile("int 173");
		break;
	case 174:
		__asm__ volatile("int 174");
		break;
	case 175:
		__asm__ volatile("int 175");
		break;
	case 176:
		__asm__ volatile("int 176");
		break;
	case 177:
		__asm__ volatile("int 177");
		break;
	case 178:
		__asm__ volatile("int 178");
		break;
	case 179:
		__asm__ volatile("int 179");
		break;
	case 180:
		__asm__ volatile("int 180");
		break;
	case 181:
		__asm__ volatile("int 181");
		break;
	case 182:
		__asm__ volatile("int 182");
		break;
	case 183:
		__asm__ volatile("int 183");
		break;
	case 184:
		__asm__ volatile("int 184");
		break;
	case 185:
		__asm__ volatile("int 185");
		break;
	case 186:
		__asm__ volatile("int 186");
		break;
	case 187:
		__asm__ volatile("int 187");
		break;
	case 188:
		__asm__ volatile("int 188");
		break;
	case 189:
		__asm__ volatile("int 189");
		break;
	case 190:
		__asm__ volatile("int 190");
		break;
	case 191:
		__asm__ volatile("int 191");
		break;
	case 192:
		__asm__ volatile("int 192");
		break;
	case 193:
		__asm__ volatile("int 193");
		break;
	case 194:
		__asm__ volatile("int 194");
		break;
	case 195:
		__asm__ volatile("int 195");
		break;
	case 196:
		__asm__ volatile("int 196");
		break;
	case 197:
		__asm__ volatile("int 197");
		break;
	case 198:
		__asm__ volatile("int 198");
		break;
	case 199:
		__asm__ volatile("int 199");
		break;
	case 200:
		__asm__ volatile("int 200");
		break;
	case 201:
		__asm__ volatile("int 201");
		break;
	case 202:
		__asm__ volatile("int 202");
		break;
	case 203:
		__asm__ volatile("int 203");
		break;
	case 204:
		__asm__ volatile("int 204");
		break;
	case 205:
		__asm__ volatile("int 205");
		break;
	case 206:
		__asm__ volatile("int 206");
		break;
	case 207:
		__asm__ volatile("int 207");
		break;
	case 208:
		__asm__ volatile("int 208");
		break;
	case 209:
		__asm__ volatile("int 209");
		break;
	case 210:
		__asm__ volatile("int 210");
		break;
	case 211:
		__asm__ volatile("int 211");
		break;
	case 212:
		__asm__ volatile("int 212");
		break;
	case 213:
		__asm__ volatile("int 213");
		break;
	case 214:
		__asm__ volatile("int 214");
		break;
	case 215:
		__asm__ volatile("int 215");
		break;
	case 216:
		__asm__ volatile("int 216");
		break;
	case 217:
		__asm__ volatile("int 217");
		break;
	case 218:
		__asm__ volatile("int 218");
		break;
	case 219:
		__asm__ volatile("int 219");
		break;
	case 220:
		__asm__ volatile("int 220");
		break;
	case 221:
		__asm__ volatile("int 221");
		break;
	case 222:
		__asm__ volatile("int 222");
		break;
	case 223:
		__asm__ volatile("int 223");
		break;
	case 224:
		__asm__ volatile("int 224");
		break;
	case 225:
		__asm__ volatile("int 225");
		break;
	case 226:
		__asm__ volatile("int 226");
		break;
	case 227:
		__asm__ volatile("int 227");
		break;
	case 228:
		__asm__ volatile("int 228");
		break;
	case 229:
		__asm__ volatile("int 229");
		break;
	case 230:
		__asm__ volatile("int 230");
		break;
	case 231:
		__asm__ volatile("int 231");
		break;
	case 232:
		__asm__ volatile("int 232");
		break;
	case 233:
		__asm__ volatile("int 233");
		break;
	case 234:
		__asm__ volatile("int 234");
		break;
	case 235:
		__asm__ volatile("int 235");
		break;
	case 236:
		__asm__ volatile("int 236");
		break;
	case 237:
		__asm__ volatile("int 237");
		break;
	case 238:
		__asm__ volatile("int 238");
		break;
	case 239:
		__asm__ volatile("int 239");
		break;
	case 240:
		__asm__ volatile("int 240");
		break;
	case 241:
		__asm__ volatile("int 241");
		break;
	case 242:
		__asm__ volatile("int 242");
		break;
	case 243:
		__asm__ volatile("int 243");
		break;
	case 244:
		__asm__ volatile("int 244");
		break;
	case 245:
		__asm__ volatile("int 245");
		break;
	case 246:
		__asm__ volatile("int 246");
		break;
	case 247:
		__asm__ volatile("int 247");
		break;
	case 248:
		__asm__ volatile("int 248");
		break;
	case 249:
		__asm__ volatile("int 249");
		break;
	case 250:
		__asm__ volatile("int 250");
		break;
	case 251:
		__asm__ volatile("int 251");
		break;
	case 252:
		__asm__ volatile("int 252");
		break;
	case 253:
		__asm__ volatile("int 253");
		break;
	case 254:
		__asm__ volatile("int 254");
		break;
	case 255:
		__asm__ volatile("int 255");
		break;
	}
}
