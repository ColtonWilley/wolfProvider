# Building wolfProvider on Windows with Visual Studio

This directory holds the Visual Studio build of wolfProvider, laid out the same
way as every other wolfSSL-family Windows project (`wolfssh/ide/winvs`,
`wolfEngine/windows`, `wolfTPM/IDE/VisualStudio`, ...): the solution, the property
sheet and `user_settings.h` at this level, and the project itself one directory
down.

```
wolfProvider/IDE/WINVS/
    wolfprovider.sln
    wolfprovider.props          user macros only - directory names
    user_settings.h             configures wolfSSL for wolfProvider
    wolfprovider/
        wolfprovider.vcxproj
        wolfprovider.def
```

The build produces **`libwolfprov.dll`**, an OpenSSL 3.x provider module. The
filename matters: OpenSSL's `-provider libwolfprov` resolves to `libwolfprov.dll`
on Windows, so `TargetName` is set explicitly rather than defaulting to the
project name.

> **Status.** This is the first Windows port of wolfProvider. Read
> "Known limitations" at the end before relying on it.

## Prerequisites

- Visual Studio 2022, or the standalone **VS 2022 Build Tools**, with the C++
  toolset (v143) and **MASM (`ml64.exe`)**. MASM is required, not optional -
  wolfSSL's assembly sources will not build without it.
- Perl and NASM are needed only to build OpenSSL, not wolfProvider.
- x64 only. No Win32 configuration is provided.

## Directory layout - three sibling trees

The project files assume wolfProvider, wolfSSL and OpenSSL sit next to each other.
This is the family convention, and it is why the checked-in project builds with no
arguments:

```
C:\src\
    openssl\          built in place; libcrypto.lib and include\ at its root
    wolfProvider\
    wolfssl\
```

`wolfprovider.props` expresses this as `..\..\..\..\wolfssl` and
`..\..\..\..\openssl`, relative to the **project** directory. Any of its macros can
be overridden on the MSBuild command line, e.g. `/p:wolfCryptDir=D:\wolfssl`.

## Step 1 - OpenSSL

Build OpenSSL 3.x as a shared library with the **release (`/MD`) runtime**:

```
perl Configure VC-WIN64A shared --prefix=C:\out\openssl
nmake
nmake install_sw
```

wolfProvider links `libcrypto.lib` from the **source tree root**
(`$(openSslDir)`), following wolfEngine's precedent. A provider does not need
`libssl`.

## Step 2 - `user_settings.h`, and the two-copy rule

wolfProvider ships the `user_settings.h` that configures wolfSSL for it. There are
two copies and **keeping them identical is a manual obligation**:

> The file `wolfProvider\IDE\WINVS\user_settings.h` contains the settings used to
> configure wolfSSL with the appropriate settings. This file must be copied from
> the directory `wolfProvider\IDE\WINVS` to `wolfssl\IDE\WIN`. **If you change one
> copy you must change both copies.**

```
copy /Y C:\src\wolfProvider\IDE\WINVS\user_settings.h C:\src\wolfssl\IDE\WIN\user_settings.h
```

wolfProvider compiles against its own copy (via `..` on the include path);
wolfSSL's own project finds the other one through `./IDE/WIN`.

**Copy, never edit the wolfSSL-side file.** It is a derived artifact. If the two
diverge, `wolfssl.dll` and `libwolfprov.dll` are compiled against different
configurations, and any macro affecting struct layout produces **memory corruption
at the first cross-DLL call, not a build error.** Verify before every build:

```powershell
(Get-FileHash C:\src\wolfProvider\IDE\WINVS\user_settings.h).Hash -eq `
(Get-FileHash C:\src\wolfssl\IDE\WIN\user_settings.h).Hash     # must be True
```

## Step 3 - wolfSSL, built as a DLL

wolfSSL is linked **shared, always**. Windows Server runs many processes and
threads with independent consumers of these libraries; statically linking the
underlying crypto library into every consumer is the wrong architecture.

wolfSSL v5.9.2-stable needs **three command-line workarounds** on a modern
toolchain. None of them modifies wolfSSL's source tree, and all three are
deliberate: a wolfSSL version bump cannot silently drop them.

```cmd
set "ML=/DWOLFSSL_SP_384 /DWOLFSSL_SP_521 /DWOLFSSL_SP_1024 /DWOLFSSL_SP_4096"

msbuild wolfssl64.sln /t:wolfssl ^
    /p:Configuration="DLL Release" /p:Platform=x64 ^
    /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0 ^
    /p:ForceImportBeforeCppTargets=C:\src\extra-sources.props
```

**1. `PlatformToolset`.** `wolfssl.vcxproj` declares `v110` (VS 2012) in every
configuration. Without the override:
`error MSB8020: The build tools for Visual Studio 2012 ... cannot be found`.
This is what the IDE's "Retarget solution" does, without editing the file.

**2. The `ML` environment variable.** `wolfssl.vcxproj` assembles the MASM sources
with `ml64.exe /c /Zi /Fo... file.asm` and passes **no `/D`**. But
`sp_x86_64_asm.asm` is conditionally assembled on `WOLFSSL_SP_384`, `_521`,
`_1024` and `_4096`, so those routines are silently skipped and the DLL fails to
link with ~207 unresolved `sp_*` symbols. `ml64` reads default options from `ML`,
so setting it supplies the missing defines. On Linux the same macros reach the
assembler through `AM_CCASFLAGS`, which is why this only bites Windows.

> Pass **only** the four SP size macros. Do not add `HAVE_INTEL_AVX2`: the `.asm`
> files define AVX1/AVX2/`_WIN64` themselves, and `aes_gcm_asm.asm` guards
> `HAVE_INTEL_AVX2 = 1` with `IFNDEF NO_AVX2_SUPPORT` - the wrong symbol - so an
> external definition is a hard `A2008: syntax error`.

**3. A missing source file.** `wolfcrypt\src\rng_bank.c` exists on disk but is
absent from `wolfssl.vcxproj`'s source list. This configuration sets
`WC_RNG_BANK_SUPPORT`, so `random.c` calls into it and the link fails on
`wc_rng_bank_checkin`, `wc_BankRef_Release` and
`wc_local_rng_bank_checkout_for_bankref`. Inject it without editing wolfSSL:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <ClCompile Include="$(MSBuildProjectDirectory)\wolfcrypt\src\rng_bank.c" />
  </ItemGroup>
</Project>
```

Outputs land in `C:\src\wolfssl\DLL Release\x64\`. **That directory name contains a
space** - quote it in every shell command.

All three are being reported upstream; expect them to become unnecessary.

## Step 4 - wolfProvider

```cmd
msbuild IDE\WINVS\wolfprovider.sln /p:Configuration=Release /p:Platform=x64
```

No `/p:` overrides are needed with the sibling layout above. The output is:

```
IDE\WINVS\Release\x64\libwolfprov.dll
```

`LNK4221` on many objects is expected and suppressed: most of wolfProvider's 48
translation units collapse to declaration-only under any single configuration,
because their bodies sit inside feature macros. All 48 are compiled in every
configuration on purpose - excluding them would make the source list a function of
the configuration, so enabling a feature later would fail at link with a cause
buried in a project file.

## Step 5 - staging `wolfssl.dll`, which is not optional

`libwolfprov.dll` imports `wolfssl.dll`, and **the process that searches for it is
the application loading the provider, not MSBuild.** OpenSSL loads a provider with
a plain `LoadLibrary` on a fully qualified path, which locates the module itself
but does **not** add the module's directory to the search path used for the
module's own imports.

**Putting `wolfssl.dll` next to `libwolfprov.dll` does not work.** It must be
reachable from the loading executable:

- for development, prepend the directory to the process `PATH`:
  ```powershell
  $env:PATH = "C:\src\wolfssl\DLL Release\x64;$env:PATH"
  ```
- for deployment, copy `wolfssl.dll` beside the executable that loads the
  provider, or install it to a directory on the system `PATH`.

Without this the provider fails to load with a bare DSO error that looks like a
build problem. `libcrypto-3-x64.dll` needs no staging: the host process has
already loaded it, so the loader binds it by name.

## Step 6 - verifying it works

Listing the provider proves only that its entry point returned. To prove
algorithms are reachable, exercise them:

```powershell
$env:PATH = "C:\src\wolfssl\DLL Release\x64;$env:PATH"
$P = '-provider-path', 'C:\src\wolfProvider\IDE\WINVS\Release\x64', '-provider', 'libwolfprov'

openssl list -providers @P                # expect: libwolfprov, status: active
openssl list -digest-algorithms @P        # expect SHA2-256 @ libwolfprov
openssl list -cipher-algorithms @P        # expect AES-256-CBC @ libwolfprov
openssl list -kdf-algorithms @P           # expect HKDF, TLS13-KDF
openssl list -random-generators @P        # expect CTR-DRBG, HASH-DRBG

# SHA-256("abc") - a known answer, so this checks correctness not just plumbing
openssl dgst -sha256 @P abc.txt
# ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
```

The default provider is deliberately not loaded above, so anything that succeeds
was satisfied by wolfProvider.

## Configurations

`Release|x64` is the deliverable. `Debug|x64` is declared so the solution is
well-formed but is **not** currently a build target: it needs a `/MDd` OpenSSL and
wolfSSL's `DLL Debug\x64`. Never mix runtimes - `/MD` against `/MDd`, or `/MD`
against `/MT`, does not produce a link error, it produces heap corruption when an
allocation crosses a DLL boundary.

For extra logging, build with `WOLFPROV_DEBUG` defined and set:

```
set WOLFPROV_LOG_LEVEL=WP_LOG_LEVEL_ALL
set WOLFPROV_LOG_COMPONENTS=WP_LOG_COMP_ALL
```

Both variables take the symbolic names, not numbers.

## Known limitations

| | |
|---|---|
| **SEED-SRC** | POSIX-only; `src/wp_seed_src.c` is written against `/dev/urandom`. Disabled on Windows, so `SEED-SRC` is absent from `openssl list -random-generators`. The DRBG self-seeds through wolfSSL instead. |
| **Unit tests** | `test/unit.test` reaches wolfProvider's internal symbols, and the module exports only `OSSL_provider_init`. Porting it needs a larger export surface or a static-library target. Not done. |
| **Replace-default mode** | The replace-default patch loads the module as `wolfprov`, which resolves to `wolfprov.dll` rather than `libwolfprov.dll`. Broken on Windows by construction. |
| **FIPS** | Not built or tested in this configuration. |
| **`USE_INTEL_SPEEDUP`** | Deliberately omitted. wolfSSL ships x86-64 assembly for nine features as GAS `.S` only, with no MASM equivalent (`fe_x25519_asm`, `wc_mlkem_asm`, `sha3_asm`, `sha256_asm`, `sha512_asm`, `wc_mldsa_asm`, `aes_gcm_x86_asm`, `sm3_asm`, `sp_sm2_x86_64_asm`), so enabling it fails to link. `WOLFSSL_AESNI` and `WOLFSSL_SP_X86_64_ASM` are kept, so AES and the big-integer paths are still accelerated; Curve25519, Ed25519, ML-KEM, ML-DSA and SHA-2/3 run from C. **The throughput cost is not the whole story:** omitting it also makes `settings.h:4273-4279` enable `WOLFSSL_CURVE25519_BLINDING`, and that in turn exposed a latent wolfProvider defect - curve25519 keys need an RNG attached with `wc_curve25519_set_rng()`, which wolfProvider was not doing, breaking X25519 (including the default TLS 1.3 group). That is **not Windows-specific**: it reproduces in any `--disable-intelasm` build on any platform. |
| **CI** | No automated Windows build yet. |
