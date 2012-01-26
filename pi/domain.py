
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

import piw
from pi import const,utils,logic
from pibelcanto import lexicon

def convert_data(value, src_dom, sink_dom):
    if src_dom == sink_dom:
        return value

    n = src_dom.normalizer()
    d = sink_dom.denormalizer()

    return d(n(value))

def convert(value, src_dom, sink_dom):
    if src_dom == sink_dom:
        return value

    n = src_dom.normalizer()
    d = sink_dom.denormalizer()

    return sink_dom.data2value(d(n(src_dom.value2data(value))))

class Domain:
    numeric = False

    def __init__(self,hints=()):
        self.hints = hints
        self.control = None

    def __cmp__(self,other):
        if self is other:
            return 0
        if isinstance(other,str):
            return cmp(self.canonical(),other)
        if isinstance(other,Domain):
            return cmp(self.canonical(),other.canonical())
        return -1

    def __hash__(self):
        return hash(self.canonical())

    def __str__(self):
        return self.canonical()

    def normalize(self,sink):
        """
        Given a functor to transmit on using the
        fastdata array form, return a new functor which can be called
        with values from the domain
        """
        #return piw.d2d_convert(self.normalizer(),sink)
        return sink

    def denormalize(self,sink):
        """
        Given a functor to transmit on using the
        domain form, return a new functor which can be called
        with values in the fastdata array form
        """
        return piw.d2d_convert(self.denormalizer(),sink)

    def convert(self,sink_dom,sink):
        """
        Given a source domain, a sink domain, and a
        functor to transmit in the sink domain, return
        a new functor which can be called with values
        from the source domain
        """
        if self == sink_dom:
            return sink

        return piw.d2d_chain(self.normalizer(),sink_dom.denormalizer(),sink)

    def iso(self):
        return False

    def flag(self,name):
        for t in self.hints:
            if t.pred==name:
                return True
        return False

    def hint(self,name):
        for t in self.hints:
            if t.pred==name:
                return t.args
        return None

class BoundedInt(Domain):
    numeric = True

    def __init__(self,min,max,rest=None,hints=()):
        Domain.__init__(self,hints)
        self.min=int(min)
        self.max=int(max)
        if rest is None:
            if self.min<0 and self.max>0:
                self.rest=0
            else:
                self.rest=self.min
        else:
            self.rest=int(rest)

        inc=self.hint('inc')
        self.inc = inc[0] if inc is not None else 1

        biginc=self.hint('biginc')
        self.biginc = biginc[0] if biginc is not None else 10

        control=self.hint('control')
        self.control = control[0] if control is not None else 'updown'

    def normalizer(self):
        return piw.bint_normalizer(self.min,self.max,self.rest)
    def denormalizer(self):
        return piw.bint_denormalizer(self.min,self.max,self.rest)
    def data2value(self,d):
        if d.is_long():
            v=d.as_long()
        else:
            v=int(d.as_renorm(self.min,self.max,self.rest))
        if v>=self.min and v<=self.max:
            return v
        return self.rest
    def value2data(self,v,t=0L):
        v=int(v)
        if v>=self.min and v<=self.max:
            return piw.makelong_bounded(self.max,self.min,self.rest,v,t)
        raise ValueError('int value %s inappropriate for %s' % (v,self))
    def canonical(self):
        return 'bint(%d,%d,%d,%s)' % (self.min,self.max,self.rest,logic.render_term(self.hints))
    def default(self):
        return self.rest
    def up(self,val):          return self.up_by(val,self.inc)
    def down(self,val):        return self.down_by(val,self.inc)
    def up_by(self,val,inc):   return min(val+inc,self.max)
    def down_by(self,val,inc): return max(val-inc,self.min)

class Enum(Domain):
    def __init__(self,*values):
        Domain.__init__(self)
        self.values = map(int,values)
        self.values.sort()
        self.min = self.values[0]
        self.max = self.values[-1]

        control=self.hint('control')
        self.control = control[0] if control is not None else 'updown'

    def normalizer(self):
        return piw.bint_normalizer(self.min,self.max,self.min)
    def denormalizer(self):
        return piw.bint_denormalizer(self.min,self.max,self.min)
    def data2value(self,d):
        if d.is_long():
            v=d.as_long()
        else:
            v=int(d.as_renorm(self.min,self.max,self.min))
        if v in self.values:
            return v
        return self.rest
    def value2data(self,v,t=0L):
        v=int(v)
        if v in self.values:
            return piw.makelong_bounded(self.max,self.min,self.min,v,t)
        raise ValueError('int value %s inappropriate for %s' % (v,self))
    def canonical(self):
        return 'enum(%s)' % ','.join(map(str,self.values))
    def default(self):
        return self.min
    def up(self,val):
        return self.up_by(val,1)
    def down(self,val):
        return self.down_by(val,1)
    def up_by(self,val,inc):
        try:
            i=self.values.index(val)
        except:
            raise ValueError('bad value for enum',val)
        return self.values[min(i+inc,len(self.values)-1)]
    def down_by(self,val,inc):
        try:
            i=self.values.index(val)
        except:
            raise ValueError('bad value for enum',val)
        return self.values[max(i-inc,0)]

class EnumOrNull(Enum):
    def data2value(self,d):
        if d.is_null(): return None
        if d.is_bool() and not d.as_bool(): return None
        return Enum.data2value(self,d)
    def value2data(self,v,t=0L):
        if v is None: return piw.makenull(t)
        if v is False: return piw.makenull(t)
        return Enum.value2data(self,v,t)
    def canonical(self):
        return 'enumn(%s)'  % ','.join(map(str,self.values))
    def up_by(self,val,inc):
        if val is None: val=self.rest
        return Enum.up_by(self,val,inc)
    def down_by(self,val,inc):
        if val is None: val=self.rest
        return Enum.down_by(self,val,inc)
    def default(self):
        return None

class StringEnum(Domain):
    def __init__(self,*values):
        Domain.__init__(self)
        self.values=values
        control=self.hint('control')
        self.control = control[0] if control is not None else 'updown'

    def normalizer(self):
        return piw.string_normalizer()

    def denormalizer(self):
        return piw.string_denormalizer()

    def data2value(self,d):
        if d.is_string():
            v=d.as_string()
            if v in self.values:
                return v
        return self.values[0]

    def value2data(self,v,t=0L):
        if v is None: 
            raise ValueError('empty string is not in belcanto lexicon')
        e=lexicon.lexicon.get(v)
        if e is None:
            raise ValueError(v,'not in belcanto lexicon')
        v=str(v)
        return piw.makestring(v,t)

    def canonical(self):
        return 'enums(%s)' % ','.join(map(str,self.values))

    def default(self):
        return self.values[0]

    def up(self,val):
        return self.up_by(val,1)

    def down(self,val):
        return self.down_by(val,1)

    def up_by(self,val,inc):
        try:
            i=self.values.index(val)
        except:
            raise ValueError('bad value for string enum',val)
        return self.values[min(i+inc,len(self.values)-1)]

    def down_by(self,val,inc):
        try:
            i=self.values.index(val)
        except:
            raise ValueError('bad value for string enum',val)
        return self.values[max(i-inc,0)]

class BoundedFloat(Domain):
    numeric = True

    def __init__(self,min,max,rest=None,hints=(),verbinc=None):
        Domain.__init__(self,hints)
        self.min=float(min)
        self.max=float(max)
        if rest is None:
            if self.min<0 and self.max>0:
                self.rest=0
            else:
                self.rest=self.min
        else:
            self.rest=float(rest)

        inc = self.hint('inc')
        self.inc = inc[0] if inc is not None else (self.max-self.min)/100.0

        if verbinc is not None:
            self.verbinc = verbinc
        else:
            self.verbinc = self.inc

        biginc = self.hint('biginc')
        self.biginc = biginc[0] if biginc is not None else 10*self.inc

        control=self.hint('control')
        self.control = control[0] if control is not None else 'updown'

    def normalizer(self):
        return piw.bfloat_normalizer(self.min,self.max,self.rest)
    def denormalizer(self):
        return piw.bfloat_denormalizer(self.min,self.max,self.rest)
    def data2value(self,d):
        if d.is_float():
            v=d.as_renorm(self.min,self.max,self.rest)
            f=d.as_float()
            if abs(v-f)<1e-6:
                v=f
            if v>=self.min and v<=self.max:
                return v
        return self.rest
    def value2data(self,v,t=0L):
        v=float(v)
        if v>=self.min and v<=self.max:
            return piw.makefloat_bounded(self.max,self.min,self.rest,v,t)
        raise ValueError('float value %s inappropriate for %s' % (v,self))
    def canonical(self):
        return 'bfloat(%g,%g,%g,%s)' % (self.min,self.max,self.rest,logic.render_term(self.hints))
    def default(self):
        return self.rest
    def up(self,val):          return self.up_by(val,self.verbinc)
    def down(self,val):        return self.down_by(val,self.verbinc)
    def up_by(self,val,inc):   return min(val+inc,self.max)
    def down_by(self,val,inc): return max(val-inc,self.min)

class BoundedFloatOrNull(BoundedFloat):
    def data2value(self,d):
        if d.is_null(): return None
        if d.is_bool() and not d.as_bool(): return None
        return BoundedFloat.data2value(self,d)
    def value2data(self,v,t=0L):
        if v is None: return piw.makenull(t)
        if v is False: return piw.makenull(t)
        return BoundedFloat.value2data(self,v,t)
    def canonical(self):
        return 'bfloatn(%g,%g,%g,%s)' % (self.min,self.max,self.rest,logic.render_term(self.hints))
    def up_by(self,val,inc):
        if val is None: val=self.rest
        return BoundedFloat.up_by(self,val,inc)
    def down_by(self,val,inc):
        if val is None: val=self.rest
        return BoundedFloat.down_by(self,val,inc)
    def default(self):
        return None

class BoundedIntOrNull(BoundedInt):
    def data2value(self,d):
        if d.is_null(): return None
        return BoundedInt.data2value(self,d)
    def value2data(self,v,t=0L):
        if v is None: return piw.makenull(t)
        return BoundedInt.value2data(self,v,t)
    def canonical(self):
        return 'bintn(%g,%g,%g,%s)' % (self.min,self.max,self.rest,logic.render_term(self.hints))
    def up_by(self,val,inc):
        if val is None: val=self.rest
        return BoundedInt.up_by(self,val,inc)
    def down_by(self,val,inc):
        if val is None: val=self.rest
        return BoundedInt.down_by(self,val,inc)
    def default(self):
        return None

class Bool(Domain):
    def __init__(self,hints=()):
        Domain.__init__(self,hints)
        control=self.hint('control')
        self.control = control[0] if control is not None else 'toggle'
    def normalizer(self):
        return piw.bool_normalizer()
    def denormalizer(self):
        return piw.bool_denormalizer()
    def data2value(self,d):
        if d.is_bool():
            return d.as_bool()
        if d.is_null():
            return False
        return d.as_norm()!=0.0
    def value2data(self,v,t=0L):
        v=bool(v)
        return piw.makebool(v,t)
    def canonical(self):
        return 'bool(%s)' % logic.render_term(self.hints)
    def default(self):
        return False
    def up(self,val):          return not val
    def down(self,val):        return not val
    def up_by(self,val,inc):   return not val
    def down_by(self,val,inc): return not val

class Trigger(Domain):
    def __init__(self,hints=()):
        Domain.__init__(self,hints)
        control=self.hint('control')
        self.control = control[0] if control is not None else 'trigger'
    def normalizer(self):
        return piw.bool_normalizer()
    def denormalizer(self):
        return piw.bool_denormalizer()
    def data2value(self,d):
        if not d.is_long():
            return 0L
        return d.as_long()
    def value2data(self,v,t=0L):
        v=long(v)
        return piw.makelong(v,t)
    def canonical(self):
        return 'trigger(%s)' % logic.render_term(self.hints)
    def default(self):
        return False
    def up(self,val):          return not val
    def down(self,val):        return not val
    def up_by(self,val,inc):   return not val
    def down_by(self,val,inc): return not val

class String(Domain):
    def __init__(self,hints=()):
        Domain.__init__(self,hints)
        self.__choices = self.hint('choices')
        control=self.hint('control')
        self.control = control[0] if control is not None else 'selector'
    def normalizer(self):
        return piw.string_normalizer()
    def denormalizer(self):
        return piw.string_denormalizer()
    def data2value(self,d):
        if d.is_string():
            return d.as_string()
        if d.is_null():
            return ''
        if self.__choices:
            i = int(d.as_renorm(0,len(self.__choices),0))
            return self.__choices[i]
        raise ValueError('string value inappropriate')
    def value2data(self,v,t=0L):
        if v is None: return piw.makestring('',0)
        v=str(v)
        return piw.makestring(v,t)
    def canonical(self):
        return 'string(%s)' % logic.render_term(self.hints)
    def default(self):
        return ''
    def up(self,val):          return val
    def down(self,val):        return val
    def up_by(self,val,inc):   return val
    def down_by(self,val,inc): return val

class Null(Domain):
    def normalizer(self):
        return piw.null_normalizer()
    def denormalizer(self):
        return piw.null_denormalizer()
    def data2value(self,d):
        if d.is_null():
            return None
        raise ValueError('null value inappropriate')
    def value2data(self,v,t=0L):
        if v is None:
            return piw.makenull(t)
        raise ValueError('null value %s inappropriate' % str(v))
    def canonical(self):
        return 'null()'
    def default(self):
        return None
    def up(self,val):          return None
    def down(self,val):        return None
    def up_by(self,val,inc):   return None
    def down_by(self,val,inc): return None

class Aniso(Domain):
    def __init__(self,hints=()):
        Domain.__init__(self,hints)
        control=self.hint('control')
        self.control = control[0] if control is not None else None
    def data2value(self,d):
        return d
    def value2data(self,v,t=0L):
        if isinstance(v,piw.data_base): return v
        raise ValueError('value %s inappropriate' % v)
    def canonical(self):
        return 'aniso(%s)' % logic.render_term(self.hints)
    def default(self):
        return piw.makelong(0,0)
    def up(self,val):          return val
    def down(self,val):        return val
    def up_by(self,val,inc):   return val
    def down_by(self,val,inc): return val

class Iso(Domain):

    def __init__(self,source,sample_rate,buffer_size,hints=()):
        Domain.__init__(self,hints)
        self.sample_rate = int(sample_rate)
        self.buffer_size = int(buffer_size)
        self.source = source
        self.name = str(logic.make_term('iso',self.source,self.sample_rate,self.buffer_size,hints))
    def canonical(self):
        return self.name
    def default(self):
        return None
    def normalizer(self):
        return piw.null_normalizer()
    def denormalizer(self):
        return piw.null_denormalizer()
    def data2value(self,d):
        return None
    def value2data(self,v,t=0L):
        return piw.data()
    def up(self,val):          return None
    def down(self,val):        return None
    def up_by(self,val,inc):   return None
    def down_by(self,val,inc): return None
    def iso(self):
        return True

__traits=dict(null=Null,bint=BoundedInt,bintn=BoundedIntOrNull,bfloat=BoundedFloat,bfloatn=BoundedFloatOrNull,string=String,bool=Bool,trigger=Trigger,enum=Enum,enumn=EnumOrNull,enums=StringEnum,aniso=Aniso,iso=Iso)

def traits(domain):
    """
    Return traits object for a particular domain has the following attributes:

    data2value(data) -> normal value object, throws ValueError if wrong
    value2data(data) -> data object, throws ValueError if wrong
    canonical()      -> canonical form of domain name
    normalizer()     -> returns fast normalizer d2d functor
    denormalizer()   -> returns fast denormalizer d2d functor
    default()        -> a valid default normal value in the domain
    """
    try:
        term = logic.parse_term(domain)
        klass = __traits.get(term.pred)
        if not klass:
            raise ValueError("bad domain")
        return klass(*term.args)
    except:
        utils.log_exception()
        raise ValueError("bad domain "+domain)

def canonicalise(domain):
    """
    Canonicalise a domain string"
    """
    return traits(domain).canonical()
