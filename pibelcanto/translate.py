
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

from pibelcanto.lexicon import lexicon,reverse_lexicon

def notes_to_words(notes):
    def __isnumber(word):
        for w in word:
            if not w.isdigit() and w!='.':
                return False
        return True

    w2 = []
    digit = False

    for n in notes.split():
        (w,c) = reverse_lexicon.get(n,(None,None))
        if w is None:
            if __isnumber(n):
                w2.append('!'+n)
            else:
                return ''
        else:
            if c=='digit':
                if digit:
                    w2[-1] = w2[-1]+w
                else:
                    w2.append(w)
                digit = True
            else:
                w2.append(w)
                digit = False

    return ' '.join(w2)

def words_to_notes(words):
    def __isnumber(word):
        for w in word:
            if not w.isdigit() and w!='.':
                return False
        return True

    w2 = []
    for w in words.split():
        if __isnumber(w):
            w2.extend(w)
        else:
            w2.append(w)

    w3 = []
    for w in w2:
        if w.startswith('!') and __isnumber(w[1:]):
            w3.append(w[1:])
        else:
            (b,c) = lexicon.get(w.lower(),(None,None))
            if b is None:
                return ''
            w3.append(b)

    return ' '.join(w3)
