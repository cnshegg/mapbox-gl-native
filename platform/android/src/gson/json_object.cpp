#include "json_object.hpp"

#include "json_element.hpp"

#include "../java/util.hpp"

namespace mbgl {
namespace android {
namespace gson {


jni::Object<JsonObject> JsonObject::New(jni::JNIEnv& env, const std::unordered_map<std::string, mapbox::geometry::value>& values) {
    static auto javaClass = jni::Class<JsonObject>::Singleton(env);
    static auto constructor = javaClass.GetConstructor(env);
    static auto addMethod = javaClass.GetMethod<void (jni::String, jni::Object<JsonElement>)>(env, "add");

    jni::Object<JsonObject> jsonObject = javaClass.New(env, constructor);

    for (auto &item : values) {
        jsonObject.Call(env, addMethod,
            *jni::SeizeLocal(env, jni::Make<jni::String>(env, item.first)),
            *jni::SeizeLocal(env, JsonElement::New(env, item.second)));
    }

    return jsonObject;
}

template <typename F> // void (jni::String, jni::Object<gson::JsonElement>)
static void iterateEntrySet(jni::JNIEnv& env, jni::Object<JsonObject> jsonObject, F callback) {
    // Get Set<Map.Entry<String, JsonElement>>
    static auto javaClass = jni::Class<JsonObject>::Singleton(env);
    static auto method = javaClass.GetMethod<jni::Object<java::util::Set> ()>(env, "entrySet");

    auto entryArray =
        jni::SeizeLocal(env, java::util::Set::toArray<java::util::Map::Entry>(env,
            *jni::SeizeLocal(env, jsonObject.Call(env, method))));

    size_t size = entryArray->Length(env);
    for (size_t i = 0; i < size; i++) {
        auto entry = jni::SeizeLocal(env, entryArray->Get(env, i));
        if (entry) {
            callback(
                *jni::SeizeLocal(env, java::util::Map::Entry::getKey<jni::StringTag>(env, *entry)),
                *jni::SeizeLocal(env, java::util::Map::Entry::getValue<gson::JsonElement>(env, *entry)));
        }
    }
}

mapbox::geometry::property_map JsonObject::convert(jni::JNIEnv &env, jni::Object<JsonObject> jsonObject) {
    mapbox::geometry::property_map map;

    if (jsonObject) {
        iterateEntrySet(env, jsonObject, [&map, &env](jni::String jId, jni::Object<gson::JsonElement> jsonElement) {
            map[jni::Make<std::string>(env, jId)] = JsonElement::convert(env, jsonElement);
        });
    }

    return map;
}

void JsonObject::registerNative(jni::JNIEnv &env) {
    jni::Class<JsonObject>::Singleton(env);
}

} // namespace gson
} // namespace android
} // namespace mbgl