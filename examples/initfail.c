/* Don't use libscu-c's main - instead have one that never does anything useful */
int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	return 1;
}
