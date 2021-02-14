using System.IO;
using System.Linq;
using System.Web;

namespace SampleGen
{
    static class Program
    {
        static string FilePath(string path, string name, string ext)
        => $"{path}/{name}.{ext}";

        static string EmbedSample(this string template, string id, string path)
        => template.Replace($"$$${id}$$$", HttpUtility.HtmlEncode(File.ReadAllText(path)));

        static string EmbedLanguageSamples(this string template, string path, string tag, string ext)
        => template.EmbedSample($"simple-list-{tag}", FilePath(path, "PrintSimple", ext))
            .EmbedSample($"detailed-list-{tag}", FilePath(path, "PrintDetailed", ext))
            .EmbedSample($"simple-playback-{tag}", FilePath(path, "RenderSimple", ext))
            .EmbedSample($"simple-record-{tag}", FilePath(path, "CaptureSimple", ext))
            .EmbedSample($"advanced-playback-{tag}", FilePath(path, "RenderAdvanced", ext))
            .EmbedSample($"advanced-record-{tag}", FilePath(path, "CaptureAdvanced", ext))
            .EmbedSample($"full-duplex-{tag}", FilePath(path, "FullDuplex", ext))
            .EmbedSample($"aggregate-{tag}", FilePath(path, "Aggregate", ext));

        static string EmbedAllSamples(this string templateText, string path)
        => templateText.EmbedLanguageSamples($"{path}/java/sample/xt/sample", "java", "java")
            .EmbedLanguageSamples($"{path}/net/sample", "net", "cs")
            .EmbedLanguageSamples($"{path}/cpp/sample", "cpp", "cpp");

        static void Main(string[] args)
        => File.WriteAllText(args[0], File.ReadAllText(args[1]).EmbedAllSamples(args[2]));
    }
}