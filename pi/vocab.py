
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

vocabulary = {}

def check_words(klass, words, allow_none=False):
    for w in words:
        if w is None:
            if allow_none:
                continue
            print 'vocab warning: None used as %s' % (w,klass)

        d = vocabulary.get(w)
        if d is None:
            print 'vocab warning: %s not in vocabulary (used as %s)' % (w,klass)
            continue
        if d[1] != klass:
            print 'vocab warning: %s used as %s (defined as %s)' % (w,klass,d[1])

def check_word(klass, *words):
    check_words(klass,words)

def add_word(english, music, klass):
    if english in vocabulary:
        raise ValueError('%s already defined in vocabulary' % english)
    vocabulary[english] = (music,klass)

def add_words(lex):
    for (e,(m,t)) in lex.iteritems():
        add_word(e,m,t)

add_words(lexicon.lexicon)
