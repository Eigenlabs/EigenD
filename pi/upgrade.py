
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

import re,heapq,collections
import picross

def compare_version(a,b):
    a=map(int,a.split('.'))
    b=map(int,b.split('.'))
    return cmp(a,b)

class Graph:
    def __init__(self):
        self.G = collections.defaultdict(dict)
        self.E = collections.defaultdict(dict)

    def add_edge(self, va, vb, cost, edge, way):
        if way == -1: (vb, va) = (va, vb)
        self.G[va][vb] = edge
        if not way: self.G[vb][va] = edge
        self.E[edge] = cost

    def get_edges(self, path):
        edges = []
        for ix in xrange(len(path) - 1):
            edges.append(self.G[path[ix]][path[ix + 1]])
        return edges

    def find_path(self, start, end):
        q = [(0, start, ())]  # Heap of (cost, path_head, path_rest).
        visited = set()       

        push, pop, add = heapq.heappush, heapq.heappop, visited.add
        G, E = self.G, self.E

        while q:
           (cost, v1, path) = pop(q)
           if v1 not in visited:
              add(v1)
              path += (v1,)

              if v1 == end: return path

              for (v2, e) in G[v1].iteritems():
                  if v2 not in visited:
                    push(q, (cost + E[e], v2, path))

        return []

    def find_edges(self, start, end):
        return self.get_edges(self.find_path(start,end))


class Upgrader:
    def upgrade(self,oldcversion,newcversion,tools,address,phase):
        if phase == 0:
            return self.preupgrade(tools,address)

        if phase == 1:
            if oldcversion == newcversion:
                return True
            return self.phase1(oldcversion,newcversion,tools,address)

        if phase == 2:
            if oldcversion == newcversion:
                return True
            return self.phase2(oldcversion,newcversion,tools,address)

        if phase == 3:
            return self.postupgrade(tools,address)

        return False


    def build_graph(self,oldcversion,newcversion):
        regexp1 = re.compile('^upgrade_([0-9]+_[0-9]+)_to_([0-9]+_[0-9]+)$')
        regexp2 = re.compile('^upgrade_([0-9]+_[0-9]+_[0-9]+)_to_([0-9]+_[0-9]+_[0-9]+)$')
        graph = Graph()

        highest = None

        for a in dir(self):
            m = re.match(regexp1,a) or re.match(regexp2,a)

            if m:
                old = m.groups()[0].replace('_','.')
                new = m.groups()[1].replace('_','.')
                graph.add_edge(old,new,1,(old,new,getattr(self,a)),1)
                if highest is None or compare_version(new,highest)>0:
                    highest = new

        if highest is None:
            highest = oldcversion

        graph.add_edge(highest,'99.0',1,(highest,'99.0',lambda tools,address: True),1)
        return graph

    def phase1(self,oldcversion,newcversion,tools,address):
        graph = self.build_graph(oldcversion,newcversion)

        if oldcversion!=newcversion:
            path = graph.find_edges(oldcversion,newcversion)

            if not path:
                return False

            for (a,b,u) in path:
                if u(tools,address)==False:
                    return False

        return True

    def phase2(self,oldcversion,newcversion,tools,address):
        graph = self.build_graph(oldcversion,newcversion)

        if oldcversion!=newcversion:
            path = graph.find_edges(oldcversion,newcversion)

            if not path:
                return False

            for (a,b,u) in path:
                p2 = 'phase2_%s' % b.replace('.','_')
                if hasattr(self,p2):
                    if getattr(self,p2)(tools,address)==False:
                        return False

        return True

    def postupgrade(self,tools,address):
        return True

    def preupgrade(self,tools,address):
        return True

