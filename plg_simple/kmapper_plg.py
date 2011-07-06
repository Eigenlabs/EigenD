
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

from pi import agent,atom,action,bundles,domain,paths,policy,async,logic,const,node,upgrade,utils,container,timeout,paths
from plg_simple import kmapper_version as version
from pi.logic.shortcuts import *
import piw
import picross

def removelist(l1,s):
    l2=[]
    for l in l1:
        if l not in s:
            l2.append(l)
    return l2
    
class Controller:
    def __init__(self,cookie):
        t = piw.tsd_time()
        self.__controller_dict = {}
        self.__state = node.Server(change=self.__change_controller)
        self.__filter = piw.function1(False,1,1,piw.makenull(0),cookie)
        self.__setup()

    def state(self):
        return self.__state

    def cookie(self):
        return self.__filter.cookie()

    def __change_controller(self,d):
        if d.is_dict():
            for x in ('courseoffset','courselen'):
                v = d.as_dict_lookup(x)
                if v.is_null(): self.__controller_dict.pop(x,None)
                else: self.__controller_dict[x]=v
            self.__setup()
            self.__state.set_data(utils.makedict(self.__controller_dict,d.time()))

    def __setup(self):
        d = utils.makedict(self.__controller_dict,piw.tsd_time())
        self.__filter.set_functor(piw.dict_merger(d))

    def set_course_semis(self,idx,val):
        self.ensure(idx)
        l = [i.as_float() for i in self.getlist('courseoffset')]
        l[idx-1] = float(val)
        self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

    def set_course_steps(self,idx,val):
        self.ensure(idx)
        l = [i.as_float() for i in self.getlist('courseoffset')]
        l[idx-1] = float(val+10000 if val else 0)
        self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

    def num_courses(self):
        l = self.getlist('courseoffset')
        return len(l) if l else 0

    def ensure(self,course):
        l = [i.as_float() for i in self.getlist('courseoffset')]
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

        l = [i.as_long() for i in self.getlist('courselen')]
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courselen',[piw.makelong(i,0) for i in l])

    def setlist(self,name,l):
        self.__controller_dict[name] = utils.maketuple(l,piw.tsd_time())
        self.__setup()
        self.__state.set_data(utils.makedict(self.__controller_dict,0))

    def getlist(self,name):
        d = self.__controller_dict.get(name)
        if d is None or not d.is_tuple():
            return []
        return utils.tuple_items(d)

class VirtualCourse(atom.Atom):
    def __init__(self,controller):
        self.__controller = controller
        atom.Atom.__init__(self,names='course',protocols='virtual')

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o)
        if a or o is None: return '[]'
        o=int(o)
        if o<1 or o>self.__controller.num_courses(): return '[]'
        return '[ideal([~server,course],%d)]'%o

class VirtualKey(atom.Atom):
    def __init__(self,size):
        atom.Atom.__init__(self,names='k',protocols='virtual')
        self.__size=size

    def __key(self,*keys):
        x=','.join(['cmp([dsc(~(a)"#1","%(k)d"),dsc(~(a)"#2","%(k)d"),dsc(~(a)"#3","%(k)d"),dsc(~(a)"#4","%(k)d"),dsc(~(a)"#8","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o),'cur size is',self.__size()
        if not a and o is None: return self.__key(*range(1,1+self.__size()))
        if a or o is None: return '[]'
        o=int(o)
        print 'key',o
        if o<1 or o>self.__size(): return '[]'
        return self.__key(o)

class VirtualChannel(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,names='channel',protocols='virtual')

    def __output(self,o):
        x = ','.join(['dsc(~(a)"#%d","%d")' % (x,o) for x in (2,3,4,5,9,10,11,12,23)])
        return '[cmp([%s])]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a or o is None: return '[]'
        o=int(o)
        return self.__output(o)


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='kmapper', icon='plg_simple/kgroup.png',container=17,ordinal=ordinal)

        self.__cur_size = 0
        self.__choices=[]
        self.__coursekeys=[]
        self.__course=1

        self.__private = node.Server(value=piw.makestring('[]',0),change=self.__set_members)
        self.set_private(self.__private)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self.mapper = piw.kgroup_mapper()

        self[1] = bundles.Output(1,False, names='light output',protocols='revconnect')

        self.loutput = bundles.Splitter(self.domain,self[1])
        self.lmapper = piw.clone(True)
        self.lmapper.set_filtered_output(1,self.loutput.cookie(),self.mapper.light_filter())
        self.lmux = piw.multiplexer(self.lmapper.cookie(),self.domain)

        self.linput = bundles.VectorInput(self.lmux.get_input(1),self.domain,signals=(1,))

        self[2] = atom.Atom(names='light input',protocols='revconnect',policy=self.linput.vector_policy(1,False,False),domain=domain.Aniso())

        self[3] = bundles.Output(1,False, names='strip position output',ordinal=1)
        self[4] = bundles.Output(2,False, names='strip position output',ordinal=2)
        self[5] = bundles.Output(3,False, names='breath output')

        self.soutput = bundles.Splitter(self.domain,self[3],self[4],self[5])
        self.sgate = piw.gate(self.soutput.cookie(),True)
        self.sinput = bundles.VectorInput(self.sgate.cookie(),self.domain,signals=(1,2,3))

        self[6] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.sinput.vector_policy(1,False),names='strip position input',ordinal=1)
        self[7] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.sinput.vector_policy(2,False),names='strip position input',ordinal=2)
        self[8] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.sinput.vector_policy(3,False),names='breath input')

        self[9] = bundles.Output(1,False, names='activation output')
        self[10] = bundles.Output(2,False, names='pressure output')
        self[11] = bundles.Output(3,False, names='roll output')
        self[12] = bundles.Output(4,False, names='yaw output')
        self[26] = bundles.Output(5,False, names='key output')
        self[27] = bundles.Output(6,False, names='original key output')

        self.koutput = bundles.Splitter(self.domain,self[9],self[10],self[11],self[12],self[26],self[27])
        self.kpoly = piw.polyctl(10,self.koutput.cookie(),False,5)

        self.kclone = piw.clone(True)
        self.kclone.set_policy(False)
        self.kinput = bundles.VectorInput(self.kclone.cookie(),self.domain,signals=(1,2,3,4,5,6),filter=self.mapper.key_filter())

        self[13] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(1,False), names='activation input')
        self[14] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(2,False), names='pressure input')
        self[15] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(3,False), names='roll input')
        self[16] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(4,False), names='yaw input')
        self[28] = atom.Atom(domain=domain.Aniso(), policy=self.kinput.vector_policy(5,False), names='key input')
        self[29] = atom.Atom(domain=domain.Aniso(), policy=self.kinput.vector_policy(6,False), names='original key input')

        self.kclone.set_output(1,self.kpoly.cookie())

        #self.add_verb2(2,'choose([],None,role(None,[matches([k])]),option(from,[numeric,singular]))',self.__choose2)
        #self.add_verb2(3,'choose([],None,role(None,[mass([k])]))',self.__choose3)
        #self.add_verb2(4,'clear([],None)',self.__clear)
        #self.add_verb2(5,'add([],None,role(None,[mass([k])]))',self.__add)

        self.add_verb2(7,'choose([],None,role(None,[matches([k])]),option(as,[numeric]))',self.__choose)
        self.add_verb2(8,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([semitone])]))',callback=self.__set_course_semi)
        self.add_verb2(9,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([interval])]))',callback=self.__set_course_int)
        self.add_verb2(12,'clear([],None,option(None,[mass([course])]))',self.__kclear)
        self.add_verb2(13,'add([],None,role(None,[mass([k])]),option(to,[mass([course])]))',self.__kadd)
        self.add_verb2(14,'add([],None,role(None,[mass([k])]),role(to,[mass([k])]),option(as,[mass([course])]))',self.__radd)

        self[20] = VirtualKey(lambda: self.__cur_size)

        self.statusbuffer = piw.statusbuffer(self.lmux.gate_input(),2,self.lmux.get_input(2))
        self.selgate = piw.functor_backend(1,True)
        self.kclone.set_output(2,self.selgate.cookie())

        self.statusbuffer.enable(1)
        self.kclone.enable(1,True)
        self.sgate.enable(True)

        self.__upstream_size = 0
        self.__set_poly()
        self[21] = atom.Atom(domain=domain.Aniso(),policy=atom.default_policy(self.__upstream),names='controller input')

        self[22] = VirtualChannel()
        
        self[23] = bundles.Output(1,False, names='controller output', continuous=True)
        self.coutput = bundles.Splitter(self.domain,self[23])
        self.controller = Controller(self.coutput.cookie())
        self.__private[2]=self.controller.state()
        self.cgate = piw.gate(self.controller.cookie(),True)
        self.cinput = bundles.ScalarInput(self.cgate.cookie(),self.domain,signals=(1,))
        self[24] = atom.Atom(domain=domain.Aniso(), policy=self.cinput.nodefault_policy(1,False),names='controller input')
        self[25] = VirtualCourse(self.controller)

    def __upstream(self,k):
        self.__upstream_size = self.__decode(k)
        print 'upstream:',self.__upstream_size
        self.__set_poly()

    def __set_course_semi(self,subject,course,interval):
        (typ,id) = action.crack_ideal(action.arg_objects(course)[0])
        course = int(id)
        semis = action.mass_quantity(interval)
        print 'set course',course,'semis',semis
        self.controller.set_course_semis(int(course),semis)
        return action.nosync_return()

    def __set_course_int(self,subject,course,interval):
        (typ,id) = action.crack_ideal(action.arg_objects(course)[0])
        course = int(id)
        ints = action.mass_quantity(interval)
        print 'set course',course,'ints',ints
        self.controller.set_course_steps(int(course),ints)
        return action.nosync_return()

    def __set_poly(self):
        poly = max(20,max(self.__upstream_size,len(self.__cur_mapping())))
        print 'poly set to',poly
    
    def __decode(self,k):
        try:
            if not k.is_dict(): return 0
            cl = k.as_dict_lookup('courselen')
            if not cl.is_tuple(): return 0
            k=0
            for i in cl: k+=i.as_long()
            return k
        except:
            return 0

    def __getkeys(self):
        mapping = self.__cur_mapping()
        courses = [i.as_long() for i in self.getlist('courselen')]

        if not courses:
            courses = [len(mapping)]

        coursekeys = []
        keys = [m[0] for m in self.__cur_mapping()]
        while len(courses):
            courselen = courses[0]
            coursekeys.append(keys[:courselen])
            courses = courses[1:]
            keys = keys[courselen:]

        return coursekeys

    def __setkeys(self,coursekeys):
        while len(coursekeys)>0 and not coursekeys[-1]:
            coursekeys.pop()

        self.controller.setlist('courselen',[piw.makelong(len(k),0) for k in coursekeys])

        keys = []
        for c in coursekeys:
            keys.extend(c)

        mapping = [(k,str(i+1)) for (i,k) in enumerate(keys)]
        self.__set_mapping(tuple(mapping))

    def __kclear(self,subject,course):
        if course is None:
            self.controller.setlist('courselen',[piw.makelong(0,0)])
            self.__set_mapping(())
            return

        course = int(action.mass_quantity(course))-1
        coursekeys = self.__getkeys()

        if len(coursekeys) > course:
            coursekeys[course] = []

        self.__setkeys(coursekeys)

    def __radd(self,subject,f,t,course):
        if course:
            course = int(action.mass_quantity(course))-1
        else:
            course = 0

        f = int(action.mass_quantity(f))
        t = int(action.mass_quantity(t))
        r = map(str,range(f,t+1))

        self.controller.ensure(course+1)
        coursekeys = self.__getkeys()

        while len(coursekeys) <= course:
            coursekeys.append([])

        coursekeys[course] = []

        for i in range(0,len(coursekeys)):
            coursekeys[i] = removelist(coursekeys[i],r)

        coursekeys[course].extend(r)
        self.__setkeys(coursekeys)


    def __kadd(self,subject,k,course=None):
        k = str(int(action.mass_quantity(k)))

        if course:
            course = int(action.mass_quantity(course))-1
        else:
            course = 0

        self.controller.ensure(course+1)
        coursekeys = self.__getkeys()

        while len(coursekeys) <= course:
            coursekeys.append([])

        for i in range(0,len(coursekeys)):
            if k in coursekeys[i]:
                coursekeys[i].remove(k)

        coursekeys[course].append(k)

        self.__setkeys(coursekeys)

    def __choose(self,subject,dummy,course=None):
        print 'start choosing..',self.__upstream_size
        self.__choices = []
        self.__course=1

        self.mapper.clear_mapping()
        self.mapper.activate_mapping()

        curmap = self.__cur_mapping() # [[from,to 1..N]]
        active = [str(f) for (f,t) in curmap] # all active keys

        courses = [i.as_long() for i in self.controller.getlist('courselen')]
        coursekeys=[]

        if course:
            self.__course = int(action.abstract_string(course))
        else:
            # collapse current courses into 1 course
            self.__course=1
            n=0
            for c in courses:
                n=n+c
            courses=[str(n)]
            self.controller.setlist('courselen',[piw.makelong(n,0)])

        if len(courses)>=self.__course and self.__choices==[]:
            clen=courses[self.__course-1]
            cstart=0
            for i in range(0,self.__course-1):
                cstart=cstart+courses[i]
            self.__coursekeys=active[cstart:cstart+clen] 
            print 'light course: keys=',self.__coursekeys 

        for k in range(1,self.__upstream_size+1):
            if str(k) in active:
                if str(k) in self.__coursekeys:
                    self.statusbuffer.set_status(k,status_choose_active)
                else:
                    self.statusbuffer.set_status(k,status_choose_used)
            else:
                self.statusbuffer.set_status(k,status_choose_available)

        self.statusbuffer.override(True)
        self.kclone.enable(1,False)
        self.sgate.enable(False)
        self.cgate.enable(False)
        self.selgate.set_efunctor(utils.make_change_nb(piw.slowchange(utils.changify(self.__choice))))

    def __choice(self,v):
        v=paths.path2grist(str(v))
        if v is None:
            return

        if self.__choices and v==self.__choices[-1]:
            print 'choose complete:',self.__choices,v
            self.selgate.clear_efunctor()
            self.kclone.enable(1,True)
            self.sgate.enable(True)
            self.cgate.enable(True)
            self.statusbuffer.override(False)
            self.__do_mapping()
        else:
            if not v in self.__choices:
                self.__choices.append(v)
                for k in self.__coursekeys:
                    self.statusbuffer.set_status(int(k),status_choose_used)
                self.__coursekeys=[]
                self.statusbuffer.set_status(v,status_choose_active)
                print 'chose:',v
        
    def __do_mapping(self):
        self.controller.ensure(self.__course)
        cl = [i.as_long() for i in self.controller.getlist('courselen')]
        print 'old courselen',cl

        ckeys = [str(k) for k in self.__choices]
        okeys = [m[0] for m in self.__cur_mapping()]

        s=0
        no=0
        oldcourses={}
        for courselen in cl:
            no=no+1
            e=s+courselen
            oldcourses[no]=okeys[s:e]
            s=e

        remove=[]
        for k,v in oldcourses.iteritems():
            print 'old course',k,v
            if not k ==self.__course: 
                for c in ckeys:
                    if c in v:
                       print c,'already in course',k
                       remove.append((k,c))        
        for t in remove:
            oldcourses[t[0]].remove(t[1])

        oldcourses[self.__course]=ckeys
        mapping=[]
        count=0
        for k,v in oldcourses.iteritems():
            print 'adjusted courses',k,v
            for nk in v:
                count=count+1
                mapping.append((nk,str(count)))
        
        self.__set_mapping(tuple(mapping))
        cl=[]
        for v in oldcourses.itervalues():
            cl.append(len(v))
        print 'new courselen',cl
        self.controller.setlist('courselen',[piw.makelong(i,0) for i in cl])
        print '__do_mapping  mapping:',self.__cur_mapping()

    def __set_mapping(self,mapping):
        mapper = self.mapper
        mapper.clear_mapping()

        for (src,dst) in mapping:
            mapper.set_mapping(int(src),int(dst))

        mapper.activate_mapping()
        mapstr = logic.render_term(mapping)
        self.__private.set_data(piw.makestring(mapstr,0))
        self.__cur_size = len(mapping)
        self.__set_poly()

    def __set_members(self,value):
        if value.is_string():
            mapping=logic.parse_clause(value.as_string())
            self.__set_mapping(mapping)

    def __cur_mapping(self):
        return logic.parse_clause(self.__private.get_data().as_string())

class Upgrader(upgrade.Upgrader):

    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        root = tools.get_root(address)

        root.get_node(21).erase_child(254)

        n = root.get_node(255)
        courselen = n.get_meta('courselen')
        courseoffset = n.get_meta('courseoffset')

        if courselen and courselen.is_string():
            l = eval(courselen.as_string())
            t = courselen.time()
            tpl = utils.maketuple([piw.makelong(i,t) for i in l],t)
            n.set_meta('courselen',tpl)

        if courseoffset and courseoffset.is_string():
            l = eval(courseoffset.as_string())
            t = courseoffset.time()
            tpl = utils.maketuple([piw.makefloat(i,t) for i in l],t)
            n.set_meta('courseoffset',tpl)

    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        cnode = root.ensure_node(25,255)
        cnode.ensure_node(1).set_data(piw.makestring('aniso([])',0))
        cnode.ensure_node(3).set_data(piw.makestring('virtual',0))
        cnode.ensure_node(8).set_data(piw.makestring('course',0))
        return True

    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(17).erase_children()
        return True

    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)

        ctl_conn = []
        cnode = root.ensure_node(21,255)
        cnode.ensure_node(2).set_data(piw.makestring(logic.render_termlist(ctl_conn),0))
        cnode.ensure_node(1).set_data(piw.makestring('aniso([])',0))
        cnode.ensure_node(8).set_data(piw.makestring('controller input',0))
        root.ensure_node(2,255,2).set_data(piw.makestring('',0))

        return True


agent.main(Agent,Upgrader)
