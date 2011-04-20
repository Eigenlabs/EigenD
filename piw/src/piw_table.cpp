
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <piw/piw_table.h>
#include <vector>

class d2d_const_sink_t: public piw::d2d_nb_t::sinktype_t
{
    public:
        d2d_const_sink_t(const piw::data_nb_t &v): value_(v)
        {
        }

        bool iscallable() const
        {
            return true;
        }

        piw::data_nb_t invoke(const piw::data_nb_t &v) const
        {
            return value_;
        }

    private:
        piw::data_nb_t value_;
};

class d2d_table_sink_t: public piw::d2d_nb_t::sinktype_t
{
    public:
        d2d_table_sink_t(float l, float u, float z, unsigned r, const piw::d2d_nb_t &proto): buckets_(r), values_(r+1), lx_(l), ux_(u), zx_(z)
        {
            bwidth_ = (u-l)/float(r);

            ly_ = uy_ = pop0(proto,l);
            zy_ = pop0(proto,zx_);

            for(unsigned i=0; i<=buckets_; i++)
            {
                float x = l+bwidth_*i;
                values_[i] = pop0(proto,x);
                if(values_[i]>uy_) uy_=values_[i];
                if(values_[i]<ly_) ly_=values_[i];
            }

            for(unsigned i=0; i<=buckets_; i++)
            {
                values_[i] = piw::normalise(uy_,ly_,zy_,values_[i]);
            }
        }

        float pop0(const piw::d2d_nb_t &f, float i)
        {
            return f(piw::makefloat_nb(i)).as_float();
        }

        bool iscallable() const
        {
            return true;
        }

        piw::data_nb_t invoke(const piw::data_nb_t &v) const
        {
            float *fo,*fs;
            const float *fi = v.as_array();
            unsigned l = v.as_arraylen();
            float xu = v.as_array_ubound();
            float xl = v.as_array_lbound();
            float xz = v.as_array_rest();

            piw::data_nb_t out = piw::makearray_nb(v.time(), uy_, ly_, zy_, l, &fo,&fs);

            for(unsigned i=0;i<l;i++)
            {
                float v0 = piw::denormalise(xu,xl,xz,fi[i]);
                float v1 = (v0-lx_)/bwidth_;
                unsigned v2 = (unsigned)v1;
                unsigned v3 = v2+1;
                float v4 = v1-(float)v2;
                float v5;

                if(v1<=0) v5=values_[0];
                else if(v2>=buckets_) v5=values_[buckets_];
                else v5=values_[v2]+v4*(values_[v3]-values_[v2]);

                fo[i]=v5;
            }

            *fs=fo[l-1];

            return out;
        }

    private:
        unsigned buckets_;
        pic::lckvector_t<float>::lcktype values_;
        float lx_;
        float ux_;
        float zx_;
        float ly_;
        float uy_;
        float zy_;
        float bwidth_;
};

class f2f_identity_sink_t: public pic::f2f_t::sinktype_t
{
    public:
        bool iscallable() const
        {
            return true;
        }

        float invoke(float v) const
        {
            return v;
        }
};

class f2f_table_sink_t: public pic::f2f_t::sinktype_t
{
    public:
        f2f_table_sink_t(float l, float u, unsigned r, const pic::f2f_t &proto): buckets_(r), values_(r+1), lx_(l), ux_(u)
        {
            bwidth_ = (u-l)/float(r);

            values_[0] = proto(l);
            values_[buckets_] = proto(u);

            for(unsigned i=1; i<buckets_; i++)
            {
                float x = l+bwidth_*i;
                values_[i] = proto(x);
            }
        }

        bool iscallable() const
        {
            return true;
        }

        float invoke(float v) const
        {
            float v1 = (v-lx_)/bwidth_;
            unsigned v2 = (unsigned)v1;
            float v5;
            unsigned v3 = v2+1;

            if(v1<=0)
            {
                v5=values_[0];
            }
            else if(v2>=buckets_)
            {
                v5=values_[buckets_];
            }
            else
            {
                float v4 = v1-(float)v2;
                v5=values_[v2]+v4*(values_[v3]-values_[v2]);
            }


            return v5;
        }

    private:
        unsigned buckets_;
        pic::lckvector_t<float>::lcktype values_;
        float lx_;
        float ux_;
        float bwidth_;
};

piw::d2d_nb_t piw::make_d2d_table(float domain_lower, float domain_upper, float domain_rest, unsigned resolution, const piw::d2d_nb_t &proto)
{
    d2d_table_sink_t *t = new d2d_table_sink_t(domain_lower, domain_upper, domain_rest, resolution, proto);
    return piw::d2d_nb_t(pic::ref(t));
}

piw::d2d_nb_t piw::make_d2d_const(float ubound, float lbound, float rest, float value)
{
    return piw::d2d_nb_t(pic::ref(new d2d_const_sink_t(piw::makefloat_bounded_nb(ubound, lbound, rest, value, 0))));
}

pic::f2f_t piw::make_f2f_table(float domain_lower, float domain_upper, unsigned resolution, const pic::f2f_t &proto)
{
    f2f_table_sink_t *t = new f2f_table_sink_t(domain_lower, domain_upper, resolution, proto);
    return pic::f2f_t(pic::ref(t));
}


pic::f2f_t piw::make_f2f_identity()
{
    f2f_identity_sink_t *t = new f2f_identity_sink_t;
    return pic::f2f_t(pic::ref(t));
}

