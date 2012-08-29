#!/usr/bin/env python

import sys
import re
import os
import getopt

def cname(s):
    c = re.sub(r'\W', '_', s)
    if re.search(r'^\d', c):
        c = '_' + c
    return c

def usage(prog_name, err=False):
    out = sys.stderr if err else sys.stdout
    out.write('Usage: %s [options] out_file.cc paths...\n' % prog_name)
    out.write('  Options:\n')
    out.write('    --root dir    base directory in which to look up paths\n')
    out.write('    --name name   name of the instance object to generate (default CFS)\n')
    return 2 if err else 0

def main(argv):
    instance_name = 'CFS'
    root_dir = '.'

    opts, args = getopt.getopt(argv[1:], '', ['root=', 'name='])
    for opt, val in opts:
        if opt == '--root':
            root_dir = val
        elif opt == '--name':
            instance_name = val
        else:
            sys.stderr.write('Unrecognized command-line option: "%s"\n' % opt)
            return 2

    if len(args) < 2:
        return usage(argv[0], True)

    out_fname = args[0]
    paths = args[1:]

    store = {}
    header_fname = re.sub(r'\..*$', '.h', out_fname)
    if header_fname == out_fname:
        sys.stderr.write('Output file requires file extension\n')
        return 2
    c_file = open(out_fname, 'w')
    h_file = open(header_fname, 'w')
    c_file.write('#include <string.h>\n')
    c_file.write('#include "%s"\n' % os.path.basename(header_fname))
    for p in paths:
        c_name = cname(p)
        c_file.write('static const unsigned char %s[] = {' % c_name)
        src = open('%s/%s' % (root_dir, p), 'r')
        s_len = 0
        while 1:
            if s_len % 12 == 0:
                c_file.write('\n    ')
            ch = src.read(1)
            if len(ch) < 1:
                break
            s_len += 1
            c_file.write('0x%02x, ' % ord(ch))
        c_file.write('0x00\n')
        src.close()
        c_file.write('};\n')
        store[p] = (c_name, s_len)
    c_file.write('''static const struct {
    const char *fname;
    const unsigned char *data;
    const unsigned int len;
} store[] = {\n''')
    for ent in store:
        c_file.write('    {"%s", %s, %d},\n' % \
                (ent, store[ent][0], store[ent][1]))
    c_file.write('    {NULL, NULL, 0}\n')
    c_file.write('};\n')

    c_file.write('''
const unsigned char *CCFSClass::get_file(const char *fname, unsigned int *length)
{
    int i;
    for (i = 0; store[i].fname != NULL; i++)
    {
        if (strcmp(fname, store[i].fname) == 0)
        {
            if (length != NULL)
                *length = store[i].len;
            return store[i].data;
        }
    }
    return NULL;
}
''')
    h_file.write('''#ifndef CCFS_GEN_%s
#define CCFS_GEN_%s

class CCFSClass
{
public:
    const unsigned char *get_file(const char *fname, unsigned int *length);
};

CCFSClass %s;

#endif
''' % (cname(header_fname), cname(header_fname), instance_name))
    h_file.close()
    c_file.close()
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv))
