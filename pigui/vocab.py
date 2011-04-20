
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

class Vocabulary:
    def __init__(self):
        self.vocab={}
        self.english={}
        lex=lexicon.lexicon
        for (e,(m,t)) in lex.iteritems():
            if m:
                self.vocab[e]=m
                self.english[m]=e

    def matchesWord(self,notes):
        music=''
        for note in notes:
            if note!='!':
                music=music+note
 
        matches={}
        for key in self.english:
            if key[:len(music)]==music and len(key)>len(music):
                matches[key]=self.english[key]
        return len(matches)>0        

    def getWordTuples(self,words):
        tuples=[]
        for word in words:
            english='***'
            if word[1:] in self.english:
                english=self.english[word[1:]]
            tuples.append((word,english))
        return tuples
 
   
    def getMusic(self,english):
        try:
            music= '!'+self.vocab[english]
            
        except KeyError: 
            music=''
        return music

    def getEnglish(self,music):
        if music in self.english:
            return self.english[music]
        else:
            return ''

    def words_to_notes(self,str):
        words=[]
        w=[]
        w=str.split()
        
        for m in w:
            if m[0].isdigit() or m[0]=='-':
                for n in list(m):
                    words.append((self.getMusic(n),n)) 
            else:
                words.append((self.getMusic(m),m))
        
        return words


