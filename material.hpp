#ifndef material_hpp
#define material_hpp

#include "hitable.hpp"

vec3 random_in_unit_sphere();
vec3 reflect(const vec3 &v, const vec3 &n);
bool refract(const vec3 &v, const vec3 &normal, float ni_over_nt,
            vec3 &refracted);

class material {
public:
    virtual bool scatter(const ray &r_in, const hit_record &rec,
        vec3 &attenuation, ray &scattered) const = 0;
};

/****** Diffuse ******/
class lambertian : public material {
public:
    lambertian(const vec3 &albedo) : albedo(albedo) {}

    virtual bool scatter(const ray &r_in, const hit_record &rec,
                         vec3 &attenuation, ray &scattered) const {
        // hit point + normal == origin of
        // a unit sphere tangent to the hit surface
        // plus another vector to get a random pos in the unit sphere
        vec3 target = rec.p + rec.normal + random_in_unit_sphere();
        // send another ray recursively
        scattered = ray(rec.p, target-rec.p);
        attenuation = albedo;
        return true;
    }

    vec3 albedo;
};

/****** Metal ******/
class metal : public material {
public:
    metal(const vec3 &albedo, float fuzz)
        : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    virtual bool scatter(const ray &r_in, const hit_record &rec,
                         vec3 &attenuation, ray &scattered) const {
        vec3 reflected = reflect(unit(r_in.direction), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction, rec.normal) > 0);
    }

    vec3 albedo;
    float fuzz;
};

/****** Dielectric ******/
class dielectric : public material {
public:
    dielectric(float ref_idx) : ref_idx(ref_idx) {}
    virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const {

        vec3 outward_normal;
        vec3 reflected = reflect(r_in.direction, rec.normal);
        float ni_over_nt;
        attenuation = vec3(1.0, 1.0, 1.0);
        vec3 refracted;

        // Check whether light is leaving or getting into the material?
        if (dot(r_in.direction, rec.normal) > 0) {
            outward_normal = -rec.normal;
            ni_over_nt = ref_idx;
        } else {
            outward_normal = rec.normal;
            ni_over_nt = 1.0 / ref_idx;
        }

        // Check if it's total internal reflection
        if (refract(r_in.direction, outward_normal, ni_over_nt, refracted)) {
            scattered = ray(rec.p, refracted);
            return true;
        } else {
            scattered = ray(rec.p, reflected);
            return false;
        }
    }

    float ref_idx;
};


// Other fuctions
vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p = 2.0*vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while (p.lengthSquared() >= 1.0);
    return p;
}


vec3 reflect(const vec3 &v, const vec3 &n) {
    return v - 2*dot(v, n) * n;
}

/*****
n * sin(x) = n_t * sin(y) -->
n^2 * (1 - cos^2(x)) = n_t^2 * (1 - cos^2(y)) -->
cos^2y = 1 - n^2 * (1-cos^2x) / n_t^2 -->
cos(y) = sqrt( 1 - ni_over_nt * ni_over_nt * (1 - cos(x) * cos(x)) )
*****/
bool refract(const vec3 &v, const vec3 &normal, float ni_over_nt,
            vec3 &refracted) {
    vec3 unit_v = unit(v);
    float dt = dot(unit_v, normal); // cos(x)
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        // calculating the refracted ray's direction using projection
        refracted = ni_over_nt * (unit_v - normal*dt) - normal*sqrt(discriminant);
        return true;
    } else {
        return false;
    }
}

#endif
