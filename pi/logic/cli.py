#!/usr/bin/python
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


import parse
import engine
import terms
import builtin
import sys

def process(ruleset,line):
    term = parse.parse_command(line)

    if term.pred == 'assert':
        ruleset.assert_rule(terms.make_rule(term.args[0], *term.args[1:]))
        return

    success = False
    results = False

    for answer in ruleset.search(*term.args):
        success = True
        if len(answer)>0:
            results=True
            print terms.render_result(answer)

    if not success:
        print "No"
    else:
        if not results:
            print "Yes"

def commandline(ruleset):
    while True:
        try:
            line = raw_input('? ').strip()

            if line == '': continue
            if line.startswith('#'): continue

            if line == 'dump':
                for r in ruleset.iterrules(line[4:].strip()):
                    print r
                continue

            process(ruleset,line)

        except builtin.LogicError,e:
            print "raised: %s" % e.term
        except KeyboardInterrupt:
            print "interrupt."
        except EOFError:
            return

def main():
    ruleset = engine.Engine()
    interactive = True

    for file in sys.argv[1:]:
        if file!='.': 
            ruleset.assert_rules(parse.parse_rulefile(file))
        else:
            interactive=False

    if interactive:
        commandline(ruleset)
