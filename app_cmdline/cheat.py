
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

from pibelcanto import lexicon

def makecheat(lex,width=5):
    wordlength = max([len(k) for k in lex])
    wordformat = '%%-%ds %%-4s' % wordlength

    keys = [ chr(x) for x in [ord('!')]+range(ord('a'),ord('z')+1) ]
    lists = dict([(x,[]) for x in keys])

    for (k,v) in lex.items():
        f = wordformat % (k,v[0])
        c1 = k[0].lower()
        if c1.isalpha():
            lists[c1].append(f)
        else:
            lists['!'].append(f)

    for k in keys:
        if lists[k]:
            lists[k].sort()
            lists[k] = [('- %s ------------------------------' % k)[0:wordlength+5],'']+lists[k]+['']
        else:
            keys.remove(k)

    cols = [[] for c in range(0,width)]
    sum = reduce(lambda x,y: x+y, [len(l) for l in lists.values()], 0)
    collen = sum/len(cols)
    cfmt = [0 for c in cols]

    for c in range(0,len(cols)):
        cnt = 0
        while keys:
            k = keys[0]
            l = lists[k]
            x = cnt+len(l)
            keys = keys[1:]
            cols[c].extend(l)
            cnt = x
            cfmt[c] = '%%-%ds' % max([len(l) for l in cols[c]])
            if x > collen: break

    x = max([len(c) for c in cols])
    for c in cols:
        while len(c) < x: c.append('')

    fmt = ' | '.join(cfmt)
    return '\n'.join([fmt % x for x in zip(*cols)])

def main():
    print makecheat(lexicon.lexicon)
