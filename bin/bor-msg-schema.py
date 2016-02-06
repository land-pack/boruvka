#!/usr/bin/env python
##
# Boruvka
# --------
# Copyright (c)2016 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of Boruvka.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##


import sys
import re

TYPES = {
    'int8'   : 'int8_t',
    'uint8'  : 'uint8_t',
    'int16'  : 'int16_t',
    'uint16' : 'uint16_t',
    'int32'  : 'int32_t',
    'uint32' : 'uint32_t',
    'int64'  : 'int64_t',
    'uint64' : 'uint64_t',
    'char'   : 'char',
    'uchar'  : 'unsigned char',
    'short'  : 'short',
    'ushort' : 'unsigned short',
    'int'    : 'int',
    'uint'   : 'unsigned int',
    'long'   : 'long',
    'ulong'  : 'unsigned long',
    'float'  : 'float',
    'double' : 'double',
}

ZEROS = {
    'int8'   : '(int8_t)0',
    'uint8'  : '(uint8_t)0',
    'int16'  : '(int16_t)0',
    'uint16' : '(uint16_t)0',
    'int32'  : '(int32_t)0',
    'uint32' : '(uint32_t)0',
    'int64'  : '(int64_t)0',
    'uint64' : '(uint64_t)0',
    'char'   : '(char)0',
    'uchar'  : '(unsigned char)0',
    'short'  : '(short)0',
    'ushort' : '(unsigned short)0',
    'int'    : '(int)0',
    'uint'   : '(unsigned int)0',
    'long'   : '(long)0',
    'ulong'  : '(unsigned long)0',
    'float'  : '0.f',
    'double' : '0.',
}

STRUCTS = {}

MAX_MEMBERS = 31
MAX_SID = 255

class Member(object):
    def __init__(self, id, name, type, default, comment):
        self.id = id
        self.name = name
        self.type = None
        self.default = default
        self.comment = comment
        self.is_arr = False
        self.struct = None

        basetype = type
        if type.endswith('[]'):
            self.is_arr = True
            basetype = type[:-2]
            if default is not None:
                print('Error: Arrays cannot have default value.', file = sys.stderr)
                sys.exit(-1)

        if basetype in TYPES:
            self.type = basetype
        elif basetype in STRUCTS:
            self.type = 'struct'
            self.struct = STRUCTS[basetype]
        else:
            print('Error: Unkown type {0}'.format(basetype), file = sys.stderr)
            sys.exit(-1)

    def genCStructMember(self, fout):
        asterix = ''
        if self.is_arr:
            asterix = '*'

        line = '    {0} {1}{2};{3}\n'
        comm = ''
        if self.comment is not None:
            comm = self.comment
        if self.type == 'struct':
            line = line.format(self.struct.name, asterix, self.name, comm)
        else:
            line = line.format(TYPES[self.type], asterix, self.name, comm)
        fout.write(line)

        if self.is_arr:
            fout.write('    int {0}_size;\n'.format(self.name))
            fout.write('    int {0}_alloc;\n'.format(self.name))

    def cDefaultVal(self):
        if self.default is not None:
            return self.default

        if self.is_arr:
            return 'NULL, 0, 0'

        if self.type == 'struct':
            return self.struct.cDefaultVal()


        return ZEROS[self.type]

    def cSchema(self, struct_name):
        foffset = '_BOR_MSG_SCHEMA_OFFSET({0}, {{0}})'.format(struct_name)

        stype = '_BOR_MSG_SCHEMA_' + self.type.upper()
        soffset = foffset.format(self.name)
        ssize_offset = '-1'
        salloc_offset = '-1'
        ssub = 'NULL'

        if self.is_arr:
            stype = '_BOR_MSG_SCHEMA_ARR_BASE + ' + stype
            ssize_offset = foffset.format('{0}_size'.format(self.name))
            salloc_offset = foffset.format('{0}_alloc'.format(self.name))

        if self.type == 'struct':
            ssub = '&schema_{0}'.format(self.struct.name)

        sline = '{{{0}, {1}, {2}, {3}, {4}}}'.format(stype, soffset,
                                                     ssize_offset,
                                                     salloc_offset, ssub)
        return sline

class Struct(object):
    def __init__(self, name, sid):
        self.name = name
        if sid.startswith('0x'):
            self.sid = int(sid, 16)
        else:
            self.sid = int(sid)
        self.members = []
        self.before = ''
        self.after = ''

        STRUCTS[self.name] = self

        if self.sid > MAX_SID:
            print('Error: Maximal struct ID is 255!', file = sys.stderr)
            sys.exit(-1)

    def addMember(self, name, type, default = None, comment = None):
        if len(self.members) == MAX_MEMBERS:
            print('Error: Exceeded maximal number of struct members ({0})!'
                    .format(MAX_MEMBERS), file = sys.stderr)
            sys.exit(-1)
        m = Member(len(self.members), name, type, default, comment)
        self.members += [m]

    def finalize(self, before):
        self.before = before

    def addAfter(self, after):
        self.after = after

    def genCStruct(self, fout):
        fout.write(self.before)
        fout.write('struct _{0} {{\n'.format(self.name))
        for m in self.members:
            m.genCStructMember(fout)
        fout.write('};\n')
        fout.write('typedef struct _{0} {0};\n'.format(self.name))
        fout.write(self.after)

    def cDefaultVal(self):
        val = [m.cDefaultVal() for m in self.members]
        val = '{ ' + ', '.join(val) + ' }'
        return val

    def genCDefault(self, fout):
        default = [m.cDefaultVal() for m in self.members]
        fout.write('static {0} ___{0}_default = '.format(self.name))
        fout.write(self.cDefaultVal())
        fout.write('\n')

    def genCSchema(self, fout):
        fields = [m.cSchema(self.name) for m in self.members]
        fields = ['    ' + x for x in fields]
        fields = ',\n'.join(fields)

        fout.write('static bor_msg_schema_field_t __{0}_fields[] = {{\n'.format(self.name))
        fout.write(fields + '\n')
        fout.write('};\n');
        fout.write('static bor_msg_schema_t schema_{0} = {{\n'.format(self.name))
        fout.write('    {0}.\n'.format(self.sid))
        fout.write('    sizeof({0}),\n'.format(self.name))
        fout.write('    sizeof(__{0}_fields) / sizeof(bor_msg_schema_field_t),\n'.format(self.name))
        fout.write('    __{0}_fields,\n'.format(self.name))
        fout.write('    &__{0}_default\n'.format(self.name))
        fout.write('};\n');
        fout.write('\n')

def parseStructs():
    structs = []
    s = None
    comment_line = None
    before = ''
    after = ''
    linebuf = ''
    for i, line in enumerate(sys.stdin):
        if s is not None:
            idx = line.find(';')
            if idx >= 0:
                comment_line = line[idx+1:].strip('\n')
                line = line[:idx]
            line = line.strip()
            sline = line.split()
        else:
            sline = line.strip().split()

        if len(sline) == 4 and sline[0] == 'msg' and sline[-1] == '{':
            s = Struct(sline[1], sline[2])
            structs += [s]
            before = linebuf
            linebuf = ''

        elif s is not None and len(sline) >= 2:
            if len(sline) == 3:
                sline[2] = sline[2].strip('}{')
                s.addMember(sline[0], sline[1], sline[2], comment_line)
            else:
                s.addMember(sline[0], sline[1], comment = comment_line)
            comment_line = None

        elif s is not None and len(sline) == 1 and sline[0] == '}':
            s.finalize(before)
            s = None

        elif s is not None and len(sline) == 0:
            continue

        elif s is not None:
            print('Error: Invalid input line {0}: {1}'.format(i + 1, line),
                  file = sys.stderr)
            sys.exit(-1)

        else:
            linebuf += line

    structs[-1].addAfter(linebuf)

    return structs

def genCStruct(structs, fout):
    for s in structs:
        s.genCStruct(fout)

def genCSchema(structs, fout):
    for s in structs:
        s.genCDefault(fout)

    fout.write('\n')
    for s in structs:
        s.genCSchema(fout)

if __name__ == '__main__':
    opts = ['--struct', '--schema']
    if len(sys.argv) != 2 or sys.argv[1] not in opts:
        print('Usage: {0} [--struct|--schema] <in.sch'.format(sys.argv[0]))
        sys.exit(-1)

    structs = parseStructs()
    if sys.argv[1] == '--struct':
        genCStruct(structs, sys.stdout)

    elif sys.argv[1] == '--schema':
        genCSchema(structs, sys.stdout)