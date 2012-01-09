
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

from pi import async,logic,rpc,paths
from . import interpreter,imperative


class SubDelegate(interpreter.Delegate):
    def __init__(self):
        pass

    def error_message(self,err):
        return

    def buffer_done(self,*a,**kw):
        return 


@async.coroutine('internal error')
def cancel_action(agent,actions):
    for a in actions:
        if logic.is_pred_arity(a,'deferred_action',2):
            (vid,vstatus) = a.args
            result = rpc.invoke_rpc(vid,'cancel','')
            yield result

    yield async.Coroutine.success()


@async.coroutine('internal error')
def create_deferred_action(actions,trigger,interp,verb,mods,roles,args,flags,text):
    sresult = imperative.run(interp,verb,mods,roles,args)

    yield sresult

    if not sresult.status():
        yield async.Coroutine.failure(*sresult.args(),**sresult.kwds())

    (verbs,) = sresult.args()

    failed = False

    for (verb,verb_args) in verbs:
        vresult = verb.defer(interp,trigger,None,*verb_args)
        yield vresult
        if not vresult.status():
            print 'deferred action failed',vresult.args(),vresult.kwds()
            failed = True
            break

        (vid,vstatus) = vresult.args()[0]
        if vstatus:
            vstatus = logic.parse_clause(vstatus,paths.make_subst(verb.id()))
        actions.append(logic.make_term('deferred_action',vid,vstatus))

    if failed:
        vresult = cancel_action(interp.get_agent(),actions)
        yield vresult
        yield async.Coroutine.failure('deferred action failed')

    yield async.Coroutine.success()


@async.coroutine('internal error')
def create_action(agent,action_term):
    if logic.is_pred_arity(action_term,'phrase',2):
        (trigger,text) = action_term.args
        actions = []
        interp = interpreter.Interpreter(agent,agent.database, SubDelegate())
        interp.set_action(lambda *a: create_deferred_action(actions,trigger,*a))
        words = text.strip().split()
        result = interp.process_block(words)
        yield result
        if not result.status():
            yield async.Coroutine.failure(*result.args(),**result.kwds())
        yield async.Coroutine.success(actions)

    yield async.Coroutine.failure('invalid action')
