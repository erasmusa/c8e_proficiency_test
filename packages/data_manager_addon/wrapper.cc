#include <napi.h>
#include <string>
#include <stdexcept>

// Include C source directly to get access to its functions.
extern "C" {
    void generate_csv(int num_records);
    void import_csv(const char* filename);
}

// C++ wrapper for the generate_csv function
Napi::Value GenerateCsvWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check if the argument is a number
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected a number as the first argument").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    int num_records = info[0].As<Napi::Number>().Int32Value();

    // Call the original C function
    generate_csv(num_records);

    return Napi::String::New(env, "CSV generation complete");
}

// C++ wrapper for the import_csv function
Napi::Value ImportCsvWrapper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Check if the argument is a string
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string as the first argument").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string filename = info[0].As<Napi::String>().Utf8Value();

    // Call the original C function
    import_csv(filename.c_str());

    return Napi::String::New(env, "CSV import complete");
}

// Initialize the addon by exporting the C++ wrappers to Node.js
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "generateCsv"), Napi::Function::New(env, GenerateCsvWrapper));
    exports.Set(Napi::String::New(env, "importCsv"), Napi::Function::New(env, ImportCsvWrapper));
    return exports;
}

// Register the addon with Node.js
NODE_API_MODULE(data_manager_addon, Init)
