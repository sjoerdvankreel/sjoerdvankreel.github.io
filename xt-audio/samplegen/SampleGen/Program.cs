using System.IO;
using System.Linq;
using System.Web;

namespace SampleGen
{
    static class Program
    {
        static string Camel(string name)
        => string.Concat(name.Split('-').Select(p => char.ToUpper(p[0]) + p.Substring(1)));

        static string FilePath(string path, string name, string ext, bool camel)
        => !camel ? $"{path}/{name}.{ext}" : path + "/" + Camel(name) + "." + ext;

        static string EmbedSample(this string template, string id, string path)
        => template.Replace($"$$${id}$$$", HttpUtility.HtmlEncode(File.ReadAllText(path)));

        static string EmbedLanguageSamples(this string template, string path, string tag, string ext, bool camel)
        => template.EmbedSample($"simple-list-{tag}", FilePath(path, "print-simple", ext, camel))
            .EmbedSample($"detailed-list-{tag}", FilePath(path, "print-detailed", ext, camel))
            .EmbedSample($"simple-playback-{tag}", FilePath(path, "render-simple", ext, camel))
            .EmbedSample($"simple-record-{tag}", FilePath(path, "capture-simple", ext, camel))
            .EmbedSample($"advanced-playback-{tag}", FilePath(path, "render-advanced", ext, camel))
            .EmbedSample($"advanced-record-{tag}", FilePath(path, "capture-advanced", ext, camel))
            .EmbedSample($"full-duplex-{tag}", FilePath(path, "full-duplex", ext, camel))
            .EmbedSample($"aggregate-{tag}", FilePath(path, "aggregate", ext, camel));

        static string EmbedAllSamples(this string templateText, string path)
        => templateText.EmbedLanguageSamples($"{path}/java-sample/com/xtaudio/xt/sample", "java", "java", true)
            .EmbedLanguageSamples($"{path}/cli-sample", "net", "cs", true)
            .EmbedLanguageSamples($"{path}/cpp-sample", "cpp", "cpp", false);

        static void Main(string[] args)
        => File.WriteAllText(args[0], File.ReadAllText(args[1]).EmbedAllSamples(args[2]));
    }
}