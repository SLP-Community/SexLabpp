import("net.http")
import("core.base.json")

function fetch_latest(target)
    local cache_dir = path.join(os.projectdir(), "lib", "PrismaUI")
    local header_path = path.join(cache_dir, "PrismaUI_API.h")
    local tag_cache = path.join(cache_dir, ".prismaui_version")
    local cached_tag = ""
    if os.isfile(tag_cache) then
        cached_tag = io.readfile(tag_cache):trim()
    end

    local tmpfile = os.tmpfile() .. ".json"
    local ok = try {
        function()
            http.download("https://api.github.com/repos/PrismaUI-SKSE/framework/releases/latest", tmpfile, {
                headers = { ["Accept"] = "application/vnd.github+json" }
            })
            return true
        end,
        catch {
            function(err)
                if os.isfile(header_path) then
                    cprint("${yellow}[PrismaUI]${reset} GitHub unreachable, using cached header")
                else
                    raise("PrismaUI: Cannot reach GitHub and no cached header exists: " .. tostring(err))
                end
            end
        }
    }
    if not ok then
        return
    end
    local release_json = json.loadfile(tmpfile)
    os.rm(tmpfile)

    local latest_tag = release_json.tag_name
    if not latest_tag then
        cprint("${yellow}[PrismaUI]${reset} Could not parse release tag, using cached header")
        return
    end
    if latest_tag == cached_tag and os.isfile(header_path) then
        cprint("${green}[PrismaUI]${reset} Already up to date (${bright}%s${reset})", latest_tag)
        return
    end

    local asset_url = nil
    for _, asset in ipairs(release_json.assets or {}) do
        if asset.name == "PrismaUI_API.h" then
            asset_url = asset.browser_download_url
            break
        end
    end
    if not asset_url then
        raise("PrismaUI: No PrismaUI_API.h asset found in release " .. latest_tag)
    end

    os.mkdir(cache_dir)
    http.download(asset_url, header_path)
    io.writefile(tag_cache, latest_tag)
    cprint("${green}[PrismaUI]${reset} Updated to ${bright}%s${reset}", latest_tag)
end