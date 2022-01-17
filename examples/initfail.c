/* Don't use libscu-c's main - instead have one that never does anything useful */
int
main(int argc, char **argv)
{
	argc = argc;
	argv = argv;
	return 1;
}
