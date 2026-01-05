#if NETSTANDARD2_0
// ReSharper disable once CheckNamespace
namespace System.Runtime.CompilerServices
{
    /// <summary>
    /// Polyfill for init-only properties in .NET Standard 2.0
    /// </summary>
    internal static class IsExternalInit
    {
    }
}
#endif
