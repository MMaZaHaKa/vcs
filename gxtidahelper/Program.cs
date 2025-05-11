using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Media;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using vcs_gxt_pseudo_comms_helper.Properties;

namespace GtaVcsGxtSimple
{
    class Program
    {
        // Ищет присвоение переменной строкового литерала, вытягивает контент между кавычками
        private static readonly Regex CodeStringRegex = new Regex(
            @"\b[A-Za-z_][A-Za-z0-9_]*\s*=\s*""(?<key>[A-Za-z0-9_]+)""",
            RegexOptions.Compiled);

        //// Ищет строку вида [KEY] или [KEY:EXTRA], где EXTRA — любая непробельная последовательность до ]
        //private static readonly Regex GxtKeyRegex = new Regex(
        //    @"^\s*\[(?<key>[A-Za-z0-9_]+)(?::(?<extra>[^\]\r\n]+))?\]\s*$",
        //    RegexOptions.Compiled);

        static async void Play(int res_id)
        {
            await Task.Run(() =>
            {
                switch (res_id)
                {
                    case 0:
                    {
                        new SoundPlayer(Resources._2).Play(); // :/
                        break;
                    }
                }
            });
        }

        static void frompseudo()
        {
            //---------------------------------------------------------------------------
            // Читаем все строки: из файла, если указан путь, иначе — из консоли
            string[] decompiledLines;
            //if (args.Length > 0 && File.Exists(args[0]))
            //{
            //    decompiledLines = File.ReadAllLines(args[0]);
            //}
            //else
            {
                Console.WriteLine("Вставьте текст (код + GXT-блоки), завершите ввод Ctrl+Z:");
                var input = new List<string>();
                string line;
                while ((line = Console.ReadLine()) != null) { input.Add(line); }
                decompiledLines = input.ToArray();
            }
            List<(int, string)> codeKeys = new List<(int, string)>(); // ida row code, parsed key (string)
            for (int i = 0; i < decompiledLines.Length; i++)
            {
                var m = CodeStringRegex.Match(decompiledLines[i]);
                if (m.Success)
                    codeKeys.Add((i + 1, m.Groups["key"].Value)); // +1 ida начинает код с 1 строчки
            }
            //---------------------------------------------------------------------------



            //---------------------------------------------------------------------------
            string[] gxtLines = File.ReadAllLines("vcs_ENGLISH.txt"); // https://github.com/Sergeanur/GXT/blob/master/VCS%20PS2/ENGLISH.txt
            var results = new List<(int LineNumber, string Key, string Text)>();

            // 4) Для каждой строки исходника ищем метку GXT и берем текст из gxtLines
            foreach (var c in codeKeys)
            {
                for (int i = 0; i < gxtLines.Length; i++)
                {
                    if (gxtLines[i].Trim().ToLower().StartsWith("[" + c.Item2.Trim().ToLower())) // our key
                    {
                        for (int j = i + 1; j < gxtLines.Length; j++) // search key string. skip comments. etc
                        {
                            if ((gxtLines[j].Trim() != "") && !(gxtLines[j].StartsWith("["))) { results.Add((c.Item1, c.Item2, gxtLines[j].Trim())); break; }
                        }
                        break;
                    }
                }
            }
            //---------------------------------------------------------------------------



            //---------------------------------------------------------------------------
            // Выводим
            Console.Clear();
            if (results.Count == 0)
            {
                Console.WriteLine("Совпадений GXT-ключей не найдено.");
            }
            else
            {
                Console.WriteLine("Найденные GXT-блоки:");
                foreach (var (ln, key, txt) in results) { string s = $"{ln,4}: {key},  {txt}"; Console.WriteLine(s); File.AppendAllText("out.txt", s + "\r\n"); }
            }
            //---------------------------------------------------------------------------

        }

        static void livetranslate()
        {
            string lastClipboard = Clipboard.ContainsText() ? Clipboard.GetText() : "";
            string[] gxtLines = File.ReadAllLines("vcs_ENGLISH.txt"); // https://github.com/Sergeanur/GXT/blob/master/VCS%20PS2/ENGLISH.txt
            if (gxtLines.Length == 0) { return; }
            while (true)
            {
                try
                {
                    if (Clipboard.ContainsText())
                    {
                        string text = Clipboard.GetText();
                        if (!string.Equals(text, lastClipboard) && text.Trim() != "")
                        {
                            lastClipboard = text;
                            //----------------------------------------------------- (on change text)

                            Match m = CodeStringRegex.Match(text.Trim());
                            if (m.Success) { text = m.Groups["key"].Value.Trim(); }
                            else { text = text.Replace("\"", "").Replace("'", "").Trim(); }

                            if (text != "")
                            {
                                for (int i = 0; i < gxtLines.Length; i++)
                                {
                                    if (gxtLines[i].Trim().ToLower().StartsWith("[" + text.Trim().ToLower())) // our key
                                    {
                                        for (int j = i + 1; j < gxtLines.Length; j++) // search key string. skip comments. etc
                                        {
                                            if ((gxtLines[j].Trim() != "") && !(gxtLines[j].StartsWith("[")))
                                            {
                                                string out_text = gxtLines[j].Trim();
                                                Clipboard.SetText(out_text);
                                                Console.WriteLine($"{text} => {out_text}");
                                                lastClipboard = out_text;
                                                Play(0);
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }

                            //-----------------------------------------------------
                        }
                    }
                }
                catch { }
                System.Threading.Thread.Sleep(10);
            }
        }

        [STAThread] // для clipboard
        static void Main(string[] args)
        {
            //frompseudo();
            livetranslate();
        }
    }
}